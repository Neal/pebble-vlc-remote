#include <pebble.h>
#include "progress_bar.h"

static Window *window;

static ActionBarLayer *action_bar;

static GBitmap *action_icon_volume_up;
static GBitmap *action_icon_volume_down;
static GBitmap *action_icon_play;
static GBitmap *action_icon_pause;
static GBitmap *action_icon_forward;
static GBitmap *action_icon_rewind;

static TextLayer *title_layer;
static TextLayer *status_text_layer;
static TextLayer *status_layer;
static TextLayer *volume_text_layer;
static TextLayer *volume_layer;

static ProgressBarLayer *volume_bar;
static ProgressBarLayer *seek_bar;

static bool controlling_volume;

enum {
	KEY_REQUEST,
	KEY_TITLE,
	KEY_STATUS,
	KEY_VOLUME,
	KEY_SEEK,
	KEY_CONTROLLING_VOLUME,
};

static void out_sent_handler(DictionaryIterator *sent, void *context) {
	// outgoing message was delivered
}

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Failed to send AppMessage to Pebble");
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
	Tuple *title_tuple = dict_find(iter, KEY_TITLE);
	Tuple *status_tuple = dict_find(iter, KEY_STATUS);
	Tuple *volume_tuple = dict_find(iter, KEY_VOLUME);
	Tuple *seek_tuple = dict_find(iter, KEY_SEEK);

	if (title_tuple) {
		text_layer_set_text(title_layer, title_tuple->value->cstring);
	}
	if (status_tuple) {
		text_layer_set_text(status_layer, status_tuple->value->cstring);
		action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, (strcmp(status_tuple->value->cstring, "Playing") == 0) ? action_icon_pause : action_icon_play);
	}
	if (volume_tuple) {
		static char vol[5];
		snprintf(vol, sizeof(vol), "%d%%", volume_tuple->value->int16);
		text_layer_set_text(volume_layer, vol);
		progress_bar_layer_set_value(volume_bar, volume_tuple->value->int16);
	}
	if (seek_tuple) {
		progress_bar_layer_set_value(seek_bar, seek_tuple->value->int16);
	}
}

void in_dropped_handler(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Incoming AppMessage from Pebble dropped");
}

void send_request(char *request) {
	Tuplet request_tuple = TupletCString(KEY_REQUEST, request);

	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	if (iter == NULL) {
		return;
	}

	dict_write_tuplet(iter, &request_tuple);
	dict_write_end(iter);

	app_message_outbox_send();
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request(controlling_volume ? "volume_up" : "seek_forward_short");
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request(controlling_volume ? "volume_down" : "seek_rewind_short");
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request("play_pause");
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request(controlling_volume ? "volume_max" : "seek_forward_long");
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request(controlling_volume ? "volume_min" : "seek_rewind_long");
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	controlling_volume = !controlling_volume;
	action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, controlling_volume ? action_icon_volume_up : action_icon_forward);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, controlling_volume ? action_icon_volume_down : action_icon_rewind);
}

static void click_config_provider(void *context) {
	const uint16_t repeat_interval_ms = 1000;
	window_single_repeating_click_subscribe(BUTTON_ID_UP, repeat_interval_ms, up_single_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, down_single_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_SELECT, repeat_interval_ms, select_single_click_handler);
	window_long_click_subscribe(BUTTON_ID_UP, 700, up_long_click_handler, NULL);
	window_long_click_subscribe(BUTTON_ID_DOWN, 700, down_long_click_handler, NULL);
	window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	send_request("refresh");
}

static void window_load(Window *window) {
	action_bar = action_bar_layer_create();
	action_bar_layer_add_to_window(action_bar, window);
	action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

	action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, controlling_volume ? action_icon_volume_up : action_icon_forward);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, controlling_volume ? action_icon_volume_down : action_icon_rewind);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_play);

	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	title_layer = text_layer_create((GRect) { .origin = { 5, 0 }, .size = { bounds.size.w - 28, 52 } });
	text_layer_set_text(title_layer, "VLC Remote");
	text_layer_set_text_color(title_layer, GColorBlack);
	text_layer_set_background_color(title_layer, GColorClear);
	text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

	status_text_layer = text_layer_create((GRect) { .origin = { 5, 54 }, .size = bounds.size });
	text_layer_set_text(status_text_layer, "Status:");
	text_layer_set_text_color(status_text_layer, GColorBlack);
	text_layer_set_background_color(status_text_layer, GColorClear);
	text_layer_set_font(status_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));

	status_layer = text_layer_create((GRect) { .origin = { 8, 54 + 15 }, .size = bounds.size });
	text_layer_set_text(status_layer, "Unknown");
	text_layer_set_text_color(status_layer, GColorBlack);
	text_layer_set_background_color(status_layer, GColorClear);
	text_layer_set_font(status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));

	volume_text_layer = text_layer_create((GRect) { .origin = { 5, 108 }, .size = bounds.size });
	text_layer_set_text(volume_text_layer, "Volume:");
	text_layer_set_text_color(volume_text_layer, GColorBlack);
	text_layer_set_background_color(volume_text_layer, GColorClear);
	text_layer_set_font(volume_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));

	volume_layer = text_layer_create((GRect) { .origin = { 58, 102 }, .size = bounds.size });
	text_layer_set_text(volume_layer, "0%");
	text_layer_set_text_color(volume_layer, GColorBlack);
	text_layer_set_background_color(volume_layer, GColorClear);
	text_layer_set_font(volume_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

	volume_bar = progress_bar_layer_create((GRect) { .origin = { 110, 58 }, .size = { 8, 76 } });
	progress_bar_layer_set_orientation(volume_bar, ProgressBarOrientationVertical);
	progress_bar_layer_set_range(volume_bar, 0, 200);
	progress_bar_layer_set_frame_color(volume_bar, GColorBlack);
	progress_bar_layer_set_bar_color(volume_bar, GColorBlack);

	seek_bar = progress_bar_layer_create((GRect) { .origin = { 5, 138 }, .size = { 115, 8 } });
	progress_bar_layer_set_orientation(seek_bar, ProgressBarOrientationHorizontal);
	progress_bar_layer_set_range(seek_bar, 0, 100);
	progress_bar_layer_set_frame_color(seek_bar, GColorBlack);
	progress_bar_layer_set_bar_color(seek_bar, GColorBlack);

	layer_add_child(window_layer, text_layer_get_layer(title_layer));
	layer_add_child(window_layer, text_layer_get_layer(status_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(status_layer));
	layer_add_child(window_layer, text_layer_get_layer(volume_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(volume_layer));
	layer_add_child(window_layer, volume_bar);
	layer_add_child(window_layer, seek_bar);
}

static void window_unload(Window *window) {
	progress_bar_layer_destroy(volume_bar);
	progress_bar_layer_destroy(seek_bar);
	gbitmap_destroy(action_icon_volume_up);
	gbitmap_destroy(action_icon_volume_down);
	gbitmap_destroy(action_icon_play);
	gbitmap_destroy(action_icon_pause);
	gbitmap_destroy(action_icon_forward);
	gbitmap_destroy(action_icon_rewind);
	text_layer_destroy(title_layer);
	text_layer_destroy(status_text_layer);
	text_layer_destroy(status_layer);
	text_layer_destroy(volume_text_layer);
	text_layer_destroy(volume_layer);
	action_bar_layer_destroy(action_bar);
}

static void app_message_init(void) {
	app_message_open(96 /* inbound_size */, 64 /* outbound_size */);
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);
}

static void init(void) {
	action_icon_volume_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_UP);
	action_icon_volume_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_DOWN);
	action_icon_play = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY);
	action_icon_pause = gbitmap_create_with_resource(RESOURCE_ID_ICON_PAUSE);
	action_icon_forward = gbitmap_create_with_resource(RESOURCE_ID_ICON_FORWARD);
	action_icon_rewind = gbitmap_create_with_resource(RESOURCE_ID_ICON_REWIND);

	app_message_init();

	controlling_volume = persist_exists(KEY_CONTROLLING_VOLUME) ? persist_read_bool(KEY_CONTROLLING_VOLUME) : true;

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(window, true /* animated */);

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void deinit(void) {
	persist_write_bool(KEY_CONTROLLING_VOLUME, controlling_volume);
	tick_timer_service_unsubscribe();
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
