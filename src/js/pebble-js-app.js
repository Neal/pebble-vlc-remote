var messageQueue = [];

function sendNextMessage() {
	if (messageQueue.length > 0) {
		console.log('sending message to pebble: ' + JSON.stringify(messageQueue[0]));
		Pebble.sendAppMessage(messageQueue[0], appMessageAck, appMessageNack);
	}
}

function appMessageAck(e) {
	messageQueue.shift();
	sendNextMessage();
}

function appMessageNack(e) {
	console.log("Message rejected by Pebble! " + e.error);
}

function makeRequestToVLC(server_host, server_pass, request) {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', 'http://' + server_host + '/requests/status.json?' + request, true, '', server_pass);
	xhr.onload = function(e) {
		if (xhr.readyState == 4) {
			if (xhr.status == 200) {
				res = JSON.parse(xhr.responseText);

				if (res.information && res.information.category && res.information.category.meta && res.information.category.meta.filename) {
					title = res.information.category.meta.filename.substring(0,30);
				} else {
					title = 'VLC Remote';
				}

				if (res.state) {
					status = res.state;
					status = status.charAt(0).toUpperCase() + status.slice(1);
				} else {
					status = 'Unknown';
				}

				if (res.volume) {
					volume = (res.volume / 512) * 200;
					if (volume > 200) volume = 200;
					volume = Math.round(volume);
					volume = volume.toString() + '%';
				} else {
					volume = '0%';
				}

				messageQueue.push({'title': title});
				messageQueue.push({'status': status});
				messageQueue.push({'volume': volume});
			} else {
				console.log('Request returned error code ' + xhr.status.toString());
				messageQueue.push({'title': 'Error: ' + xhr.statusText});
			}
		}
		sendNextMessage();
	}
	xhr.onerror = function() {
		messageQueue.push({'title': 'Error: Failed to connect!'});
		sendNextMessage();
	};
	xhr.send(null);
}

Pebble.addEventListener('ready', function(e) {});

Pebble.addEventListener('appmessage', function(e) {
	console.log('message received: ' + JSON.stringify(e.payload));
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
				makeRequestToVLC(e.payload.server_host, e.payload.server_pass, '');
				break;
		}
	} else {
		console.log('server_host, server_pass, or request not set');
	}
});

Pebble.addEventListener('showConfiguration', function(e) {
	var options = JSON.parse(localStorage.getItem('options'));
	if (options === null) options = {};
	console.log('read options: ' + JSON.stringify(options));
	if (!options['server_host']) options['server_host'] = '';
	if (!options['server_pass']) options['server_pass'] = '';
	var uri = 'https://rawgithub.com/Neal/pebble-vlc-remote/master/html/configuration.html?' +
				'server_host=' + encodeURIComponent(options['server_host']) +
				'&server_pass=' + encodeURIComponent(options['server_pass']);
	console.log('showing configuration at uri: ' + uri);
	Pebble.openURL(uri);
});

Pebble.addEventListener('webviewclosed', function(e) {
	console.log('configuration closed');
	if (e.response) {
		var options = JSON.parse(decodeURIComponent(e.response));
		console.log('storing options: ' + JSON.stringify(options));
		localStorage.setItem('options', JSON.stringify(options));
		messageQueue.push({'server_host': options['server_host']});
		messageQueue.push({'server_pass': options['server_pass']});
		sendNextMessage();
	} else {
		console.log('no options received');
	}
});
