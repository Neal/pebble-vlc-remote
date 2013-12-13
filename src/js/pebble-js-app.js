var maxTriesForSendingAppMessage = 3;
var timeoutForAppMessageRetry = 3000;
var timeoutForHTTPRequest = 12000;

var serverHost = localStorage.getItem('server_host') || '';
var serverPass = localStorage.getItem('server_pass') || '';

function sendAppMessage(title, status, volume, seek, numTries, transactionId) {
	status   = status || 'Unknown';
	volume   = volume || 0;
	seek     = seek || 0;
	numTries = numTries || 0;
	if (numTries < maxTriesForSendingAppMessage) {
		numTries++;
		var message = { 'title': title, 'status': status, 'volume': volume, 'seek': seek };
		console.log('Sending AppMessage to Pebble: ' + JSON.stringify(message));
		Pebble.sendAppMessage(
			message, function() {}, function(e) {
				console.log('Failed sending AppMessage for transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
				setTimeout(function() {
					sendAppMessage(title, status, volume, seek, numTries, e.data.transactionId);
				}, timeoutForAppMessageRetry);
			}
		);
	} else {
		console.log('Failed sending AppMessage for transactionId:' + transactionId + '. Bailing. ' + JSON.stringify(message));
	}
}

function makeRequest(request) {
	request = request || '';
	var xhr = new XMLHttpRequest();
	xhr.open('GET', 'http://' + serverHost + '/requests/status.json?' + request, true, '', serverPass);
	xhr.timeout = timeoutForHTTPRequest;
	xhr.onload = function(e) {
		if (xhr.readyState == 4) {
			if (xhr.status == 200) {
				if (xhr.responseText) {
					res    = JSON.parse(xhr.responseText);
					title  = res.information || 'VLC Remote';
					title  = title.category || 'VLC Remote';
					title  = title.meta || 'VLC Remote';
					title  = title.filename || 'VLC Remote';
					title  = title.substring(0,30);
					status = res.state ? res.state.charAt(0).toUpperCase()+res.state.slice(1) : 'Unknown';
					status = status.substring(0,30);
					volume = res.volume || 0;
					volume = (volume / 512) * 200;
					volume = (volume > 200) ? 200 : volume;
					volume = Math.round(volume);
					length = res.length || 0;
					seek   = res.time || 0;
					seek   = (seek / length) * 100;
					seek   = Math.round(seek);
					sendAppMessage(title, status, volume, seek);
				} else {
					console.log('Invalid response received! ' + JSON.stringify(xhr));
					sendAppMessage('Error: Invalid response received!');
				}
			} else {
				console.log('Request returned error code ' + xhr.status.toString());
				sendAppMessage('Error: ' + xhr.statusText);
			}
		}
	}
	xhr.ontimeout = function() {
		console.log('HTTP request timed out');
		sendAppMessage('Error: Request timed out!');
	};
	xhr.onerror = function() {
		console.log('HTTP request return error');
		sendAppMessage('Error: Failed to connect!');
	};
	xhr.send(null);
}

Pebble.addEventListener('ready', function(e) {});

Pebble.addEventListener('appmessage', function(e) {
	console.log('AppMessage received from Pebble: ' + JSON.stringify(e.payload));

	if (!serverHost || !serverPass) {
		console.log('Server options not set!');
		sendAppMessage('Set options via Pebble app');
		return;
	}

	var request = e.payload.request || '';

	switch (request) {
		case 'play_pause':
			request = 'command=pl_pause';
			break;
		case 'volume_up':
			request = 'command=volume&val=%2B12.8';
			break;
		case 'volume_down':
			request = 'command=volume&val=-12.8';
			break;
		case 'volume_min':
			request = 'command=volume&val=0';
			break;
		case 'volume_max':
			request = 'command=volume&val=512';
			break;
		case 'seek_forward_short':
			request = 'command=seek&val=%2B10S';
			break;
		case 'seek_rewind_short':
			request = 'command=seek&val=-10S';
			break;
		case 'seek_forward_long':
			request = 'command=seek&val=%2B1M';
			break;
		case 'seek_rewind_long':
			request = 'command=seek&val=-1M';
			break;
	}

	makeRequest(request);
});

Pebble.addEventListener('showConfiguration', function(e) {
	var uri = 'http://neal.github.io/pebble-vlc-remote/?' +
				'server_host=' + encodeURIComponent(serverHost) +
				'&server_pass=' + encodeURIComponent(serverPass);
	console.log('showing configuration at uri: ' + uri);
	Pebble.openURL(uri);
});

Pebble.addEventListener('webviewclosed', function(e) {
	console.log('configuration closed');
	if (e.response) {
		var options = JSON.parse(decodeURIComponent(e.response));
		console.log('options received from configuration: ' + JSON.stringify(options));
		serverHost = options['server_host'];
		serverPass = options['server_pass'];
		localStorage.setItem('server_host', serverHost);
		localStorage.setItem('server_pass', serverPass);
		makeRequest();
	} else {
		console.log('no options received');
	}
});
