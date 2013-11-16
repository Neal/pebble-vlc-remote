#include <pebble.h>

#define SERVER_HOST "192.168.1.3:8080"
#define SERVER_PASSWORD "pass"

static Window *window;

static ActionBarLayer *action_bar;

static GBitmap *action_icon_vol_up;
static GBitmap *action_icon_vol_down;
static GBitmap *action_icon_play;
static GBitmap *action_icon_pause;

static TextLayer *title_layer;
static TextLayer *status_text_layer;
static TextLayer *status_layer;
static TextLayer *volume_text_layer;
static TextLayer *volume_layer;

static char title[30] = "VLC Remote";
static char status_text[] = "Status:";
static char status[8] = "Unknown";
static char volume_text[] = "Volume:";
static char volume[5] = "0%";

enum {
	KEY_SERVER,
	KEY_PASSWORD,
	KEY_REQUEST,
	KEY_TITLE,
	KEY_STATUS,
	KEY_VOLUME,
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

	if (title_tuple) {
		strncpy(title, title_tuple->value->cstring, sizeof(title));
		text_layer_set_text(title_layer, title);
	}
	if (status_tuple) {
		strncpy(status, status_tuple->value->cstring, sizeof(status));
		text_layer_set_text(status_layer, status);
		if (strcmp(status, "Playing") == 0) {
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_pause);
		} else {
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_play);
		}
	}
	if (volume_tuple) {
		strncpy(volume, volume_tuple->value->cstring, sizeof(volume));
		text_layer_set_text(volume_layer, volume);
	}
}

void in_dropped_handler(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "incoming message from Pebble dropped");
}

void send_request(char *request) {
	Tuplet server_tuple = TupletCString(KEY_SERVER, SERVER_HOST);
	Tuplet password_tuple = TupletCString(KEY_PASSWORD, SERVER_PASSWORD);
	Tuplet request_tuple = TupletCString(KEY_REQUEST, request);

	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	if (iter == NULL) {
		return;
	}

	dict_write_tuplet(iter, &server_tuple);
	dict_write_tuplet(iter, &password_tuple);
	dict_write_tuplet(iter, &request_tuple);
	dict_write_end(iter);

	app_message_outbox_send();
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request("vol_up");
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request("vol_down");
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request("play_pause");
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request("vol_max");
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request("vol_min");
}

static void click_config_provider(void *context) {
	const uint16_t repeat_interval_ms = 1000;
	window_single_repeating_click_subscribe(BUTTON_ID_UP, repeat_interval_ms, up_single_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, down_single_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_SELECT, repeat_interval_ms, select_single_click_handler);
	window_long_click_subscribe(BUTTON_ID_UP, 700, up_long_click_handler, NULL);
	window_long_click_subscribe(BUTTON_ID_DOWN, 700, down_long_click_handler, NULL);
}

static void window_load(Window *window) {
	action_bar = action_bar_layer_create();
	action_bar_layer_add_to_window(action_bar, window);
	action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

	action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_vol_up);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_vol_down);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_play);

	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	title_layer = text_layer_create((GRect) { .origin = { 5, 0 }, .size = { bounds.size.w - 28, 52 } });
	text_layer_set_text_color(title_layer, GColorBlack);
	text_layer_set_background_color(title_layer, GColorClear);
	text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

	status_text_layer = text_layer_create((GRect) { .origin = { 5, 54 }, .size = bounds.size });
	text_layer_set_text_color(status_text_layer, GColorBlack);
	text_layer_set_background_color(status_text_layer, GColorClear);
	text_layer_set_font(status_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));

	status_layer = text_layer_create((GRect) { .origin = { 10, 54 + 14 }, .size = bounds.size });
	text_layer_set_text_color(status_layer, GColorBlack);
	text_layer_set_background_color(status_layer, GColorClear);
	text_layer_set_font(status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));

	volume_text_layer = text_layer_create((GRect) { .origin = { 5, 100 }, .size = bounds.size });
	text_layer_set_text_color(volume_text_layer, GColorBlack);
	text_layer_set_background_color(volume_text_layer, GColorClear);
	text_layer_set_font(volume_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));

	volume_layer = text_layer_create((GRect) { .origin = { 10, 100 + 14 }, .size = bounds.size });
	text_layer_set_text_color(volume_layer, GColorBlack);
	text_layer_set_background_color(volume_layer, GColorClear);
	text_layer_set_font(volume_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));

	layer_add_child(window_layer, text_layer_get_layer(title_layer));
	layer_add_child(window_layer, text_layer_get_layer(status_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(status_layer));
	layer_add_child(window_layer, text_layer_get_layer(volume_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(volume_layer));
}

static void window_unload(Window *window) {
	text_layer_destroy(title_layer);
	text_layer_destroy(status_text_layer);
	text_layer_destroy(status_layer);
	text_layer_destroy(volume_text_layer);
	text_layer_destroy(volume_layer);
	action_bar_layer_destroy(action_bar);
}

static void app_message_init(void) {
	app_message_open(64 /* inbound_size */, 64 /* outbound_size */);
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);
}

static void init(void) {
	action_icon_vol_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_UP);
	action_icon_vol_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_DOWN);
	action_icon_play = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY);
	action_icon_pause = gbitmap_create_with_resource(RESOURCE_ID_ICON_PAUSE);

	app_message_init();

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(window, true /* animated */);

	text_layer_set_text(title_layer, title);
	text_layer_set_text(status_text_layer, status_text);
	text_layer_set_text(status_layer, status);
	text_layer_set_text(volume_text_layer, volume_text);
	text_layer_set_text(volume_layer, volume);

	send_request("refresh");
}

static void deinit(void) {
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
