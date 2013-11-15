function make_request(server, password, request) {
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

				Pebble.sendAppMessage({
					'title': title,
					'status': status,
					'volume': volume
				});
			} else {
				console.log('Request returned error code ' + xhr.status.toString());
				Pebble.sendAppMessage({'title': 'Error: ' + xhr.statusText});
			}
		}
	}
	xhr.onerror = function() {
		Pebble.sendAppMessage({'title': 'Error: Failed to connect!'});
	};
	xhr.send(null);
}

Pebble.addEventListener('ready', function(e) {});

Pebble.addEventListener('appmessage', function(e) {
	console.log('message received: ' + JSON.stringify(e.payload));
	if (e.payload.server && e.payload.password && e.payload.request) {
		switch (e.payload.request) {
			case 'play_pause':
				make_request(e.payload.server, e.payload.password, 'command=pl_pause');
				break;
			case 'vol_up':
				make_request(e.payload.server, e.payload.password, 'command=volume&val=%2B12.8');
				break;
			case 'vol_down':
				make_request(e.payload.server, e.payload.password, 'command=volume&val=-12.8');
				break;
			case 'refresh':
				make_request(e.payload.server, e.payload.password, '');
				break;
		}
	} else {
		console.log('server, password, or request not set');
	}
});
