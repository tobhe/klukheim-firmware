/*
 * Copyright (c) 2020 Tobias Heider
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <DHTesp.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <iostream>

#include "config.h"

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

#define INTERVAL_DEFAULT (50)

const int		led = 16;
int			state = 0;	// current state
int			target = 0;	// target state

/* LED PWM Fading */
const int		fadestep = 20;
unsigned long		oldmillis = 0;
unsigned long		interval = INTERVAL_DEFAULT;

/* Initialize DHT sensor */
#if 0
DHTesp dht;
#endif
/*
<form action="" method="post">
    <button name="foo" value="upvote">Upvote</button>
</form>
*/

void fade() {
	int		oldstate = state;
	unsigned long	m = millis();

	if (target == state)
		return;

	if (m - oldmillis > interval) {
		oldmillis = m;
		if (target > state) {
			state += fadestep;
			if (state > target)
				state = target;
		} else {
			state -= fadestep;
			if (state < target)
				state = target;
		}
		analogWrite(led, state);
	}
}

void handleLight() {
	String	pwm = {};
	String	message = "<form action=\"\" method=\"post\"> "
	    "<button style=\"height:50%;width:50%\">An/Aus</button></form>\n";

	if (server.method() != HTTP_POST) {
		message += "Licht ist: ";
		message += state ? "AN" : "AUS";
		message += "\n";
		server.send(200, "text/html", message);
	} else {
		message += "Licht ist: ";
		message += state ? "AN" : "AUS";
		message += "\n";
		if (!server.args()) {
			target = target ? 0 : PWMRANGE;
		}
		for (uint8_t i = 0; i < server.args(); i++) {
			if (server.argName(i) == "pwm") {
				message += "pwm: " + server.arg(i) + "\n";
				target = server.arg(i).toInt();
			}
			if (server.argName(i) == "interval") {
				message += "pwm: " + server.arg(i) + "\n";
				interval = server.arg(i).toInt();
			}
			message += "pwm: ";
			message += target;
			message += "\n";
		}
		server.send(200, "text/html", message);
	}
}

void handleRoot() {
	String message = "Test\n";
#if 0
	float humidity = dht.getHumidity();
	float temperature = dht.getTemperature();
	String message = "Sensor Wohnzimmer: ";

	message += "humidity: ";
	message += humidity;
	message += ", temperature: ";
	message += temperature;
#endif
	server.send(200, "text/plain", message);
}

void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) +
		    ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
}

void setup(void) {
	pinMode(led, OUTPUT);
	digitalWrite(led, state);
	Serial.begin(115200);
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	Serial.println("");

#if 0
	// Initialize temperature sensor
	dht.setup(16, DHTesp::DHT22);
#endif

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	if (MDNS.begin("esp8266")) {
		Serial.println("MDNS responder started");
	}

	server.on("/", handleRoot);
	server.on("/licht/", handleLight);
	server.on("/inline", []() {
		server.send(200, "text/plain", "this works as well");
	});
	server.onNotFound(handleNotFound);

	server.begin();
	Serial.println("HTTP server started");
}

void loop(void) {
	fade();
	server.handleClient();
	MDNS.update();
}
