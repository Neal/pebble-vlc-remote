var maxTriesForSendingAppMessage = 3;
var timeoutForAppMessageRetry = 3000;
var timeoutForVLCRequest = 12000;

function sendAppMessage(message, numTries, transactionId) {
	numTries = numTries || 0;
	if (numTries < maxTriesForSendingAppMessage) {
		numTries++;
		console.log('Sending AppMessage to Pebble: ' + JSON.stringify(message));
		Pebble.sendAppMessage(
			message, function() {}, function(e) {
				console.log('Failed sending AppMessage for transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
				setTimeout(function() {
					sendAppMessage(message, numTries, e.data.transactionId);
				}, 3000);
			}
		);
	} else {
		console.log('Failed sending AppMessage for transactionId:' + transactionId + '. Bailing. ' + JSON.stringify(message));
	}
}

function makeRequestToVLC(server_host, server_pass, request) {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', 'http://' + server_host + '/requests/status.json?' + request, true, '', server_pass);
	xhr.timeout = timeoutForVLCRequest;
	xhr.onload = function(e) {
		if (xhr.readyState == 4) {
			if (xhr.status == 200) {
				res = JSON.parse(xhr.responseText);

				title = res.information.category.meta.filename || 'VLC Remote';
				title = title.substring(0,30);

				status = res.state ? res.state.charAt(0).toUpperCase()+res.state.slice(1) : 'Unknown';
				status = status.substring(0,30);

				volume = res.volume || 0;
				volume = (volume / 512) * 200;
				volume = (volume > 200) ? 200 : volume;
				volume = Math.round(volume).toString() + '%';

				sendAppMessage({
					'title': title,
					'status': status,
					'volume': volume
				});
			} else {
				console.log('Request returned error code ' + xhr.status.toString());
				sendAppMessage({'title': 'Error: ' + xhr.statusText});
			}
		}
	}
	xhr.ontimeout = function() {
		sendAppMessage({'title': 'Error: Request timed out!'});
	};
	xhr.onerror = function() {
		sendAppMessage({'title': 'Error: Failed to connect!'});
	};
	xhr.send(null);
}

Pebble.addEventListener('ready', function(e) {});

Pebble.addEventListener('appmessage', function(e) {
	console.log('AppMessage received from Pebble: ' + JSON.stringify(e.payload));
	if (e.payload.server_host && e.payload.server_pass && e.payload.request) {
		switch (e.payload.request) {
			case 'play_pause':
				makeRequestToVLC(e.payload.server_host, e.payload.server_pass, 'command=pl_pause');
				break;
			case 'vol_up':
				makeRequestToVLC(e.payload.server_host, e.payload.server_pass, 'command=volume&val=%2B12.8');
				break;
			case 'vol_down':
				makeRequestToVLC(e.payload.server_host, e.payload.server_pass, 'command=volume&val=-12.8');
				break;
			case 'vol_min':
				makeRequestToVLC(e.payload.server_host, e.payload.server_pass, 'command=volume&val=0');
				break;
			case 'vol_max':
				makeRequestToVLC(e.payload.server_host, e.payload.server_pass, 'command=volume&val=512');
				break;
			case 'refresh':
			default:
				makeRequestToVLC(e.payload.server_host, e.payload.server_pass, 'refresh');
				break;
		}
	} else {
		console.log('server_host, server_pass, or request not set');
	}
});

Pebble.addEventListener('showConfiguration', function(e) {
	var options = JSON.parse(localStorage.getItem('options')) || {};
	console.log('read options: ' + JSON.stringify(options));
	var server_host = options['server_host'] || '';
	var server_pass = options['server_pass'] || '';
	var uri = 'https://rawgithub.com/Neal/pebble-vlc-remote/master/html/configuration.html?' +
				'server_host=' + encodeURIComponent(server_host) +
				'&server_pass=' + encodeURIComponent(server_pass);
	console.log('showing configuration at uri: ' + uri);
	Pebble.openURL(uri);
});

Pebble.addEventListener('webviewclosed', function(e) {
	console.log('configuration closed');
	if (e.response) {
		var options = JSON.parse(decodeURIComponent(e.response));
		console.log('storing options: ' + JSON.stringify(options));
		localStorage.setItem('options', JSON.stringify(options));
		sendAppMessage({
			'server_host': options['server_host'],
			'server_pass': options['server_pass']
		});
		setTimeout(function() {
			makeRequestToVLC(options['server_host'], options['server_pass'], 'refresh');
		}, 2000);
	} else {
		console.log('no options received');
	}
});
