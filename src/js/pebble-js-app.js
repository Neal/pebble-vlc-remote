var messageQueue = [];

function sendNextMessage() {
	if (messageQueue.length > 0) {
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

function makeRequestToVLC(server, password, request) {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', 'http://' + server + '/requests/status.json?' + request, true, '', password);
	xhr.onload = function(e) {
		if (xhr.readyState == 4) {
			if (xhr.status == 200) {
				res = JSON.parse(xhr.responseText);

				if (res.information && res.information.category && res.information.category.meta && res.information.category.meta.filename) {
					title = res.information.category.meta.filename;
				} else {
					title = 'VLC Remote';
				}

				if (res.volume) {
					volume = (res.volume / 512) * 200;
					if (volume > 200) volume = 200;
					volume = Math.round(volume);
					volume = volume.toString();
				} else {
					volume = '0%';
				}

				if (res.state) {
					status = res.state;
					status = status.charAt(0).toUpperCase() + status.slice(1);
				} else {
					status = 'Unknown';
				}

				messageQueue.push({'title': title});
				messageQueue.push({'volume': volume});
				messageQueue.push({'status': status});
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
	if (e.payload.server && e.payload.password && e.payload.request) {
		switch (e.payload.request) {
			case 'play_pause':
				makeRequestToVLC(e.payload.server, e.payload.password, 'command=pl_pause');
				break;
			case 'vol_up':
				makeRequestToVLC(e.payload.server, e.payload.password, 'command=volume&val=%2B12.8');
				break;
			case 'vol_down':
				makeRequestToVLC(e.payload.server, e.payload.password, 'command=volume&val=-12.8');
				break;
			case 'refresh':
				makeRequestToVLC(e.payload.server, e.payload.password, '');
				break;
		}
	} else {
		console.log('server, password, or request not set');
	}
});
