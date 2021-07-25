#include "esp_pm.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>

#include "../../.ssid.h"
/*
// あるいは、以下のような感じで設定を書いてください

const char* ssid = "SSID"; // アクセスポイントのSSID
const char* password = "password"; // アクセスポイントのパスワード
const char* tz = "JST-9"; // タイムゾーン
const char* ntp[] = {"0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org"}; // NTPサーバー


*/

#define JJY_40k_OUTPUT_PIN 23 // 40kHzコードを出力するピン(-1 = 使わない場合)
#define JJY_60k_OUTPUT_PIN 22 // 60kHzコードを出力するピン(-1 = 使わない場合)
#define JJY_LED_OUTPUT_PIN 2  // タイムコードを出力するLEDピン(-1 = 使わない場合)

#define LEDC_40k_CHANNEL 0	   // LEDCの40kHz用チャネル
#define LEDC_60k_CHANNEL 10	   // LEDCの60kHz用チャネル
#define LEDC_RESOLUTION_BITS 1 // LEDCの解像度

// タイムコードを作成するクラス
class jjy_timecode_generator_t
{
public:
	unsigned char year10;
	unsigned char year1;
	unsigned char yday100;
	unsigned char yday10;
	unsigned char yday1;
	unsigned short yday;
	unsigned char hour10;
	unsigned char hour1;
	unsigned char min10;
	unsigned char min1;
	unsigned char wday;
	unsigned char mon;
	unsigned char day;

	unsigned char result[61];

public:
	void generate()
	{
		unsigned char pa1 = 0;
		unsigned char pa2 = 0;
		for (int sec = 0; sec < 60; sec++)
		{
			unsigned char cc;

			cc = 0;
#define SET(X) \
	if (X)     \
	cc |= 1
			switch (sec)
			{
			case 0: /* marker */
				cc = 2;
				break;

			case 1:
				SET(min10 & 4);
				pa2 ^= cc;
				break;

			case 2:
				SET(min10 & 2);
				pa2 ^= cc;
				break;

			case 3:
				SET(min10 & 1);
				pa2 ^= cc;
				break;

			case 4:
				cc = 0;
				break;

			case 5:
				SET(min1 & 8);
				pa2 ^= cc;
				break;

			case 6:
				SET(min1 & 4);
				pa2 ^= cc;
				break;

			case 7:
				SET(min1 & 2);
				pa2 ^= cc;
				break;

			case 8:
				SET(min1 & 1);
				pa2 ^= cc;
				break;

			case 9:
				cc = 2;
				break;

			case 10:
				cc = 0;
				break;

			case 11:
				cc = 0;
				break;

			case 12:
				SET(hour10 & 2);
				pa1 ^= cc;
				break;

			case 13:
				SET(hour10 & 1);
				pa1 ^= cc;
				break;

			case 14:
				cc = 0;
				break;

			case 15:
				SET(hour1 & 8);
				pa1 ^= cc;
				break;

			case 16:
				SET(hour1 & 4);
				pa1 ^= cc;
				break;

			case 17:
				SET(hour1 & 2);
				pa1 ^= cc;
				break;

			case 18:
				SET(hour1 & 1);
				pa1 ^= cc;
				break;

			case 19:
				cc = 2;
				break;

			case 20:
				cc = 0;
				break;

			case 21:
				cc = 0;
				break;

			case 22:
				SET(yday100 & 2);
				break;

			case 23:
				SET(yday100 & 1);
				break;

			case 24:
				cc = 0;
				break;

			case 25:
				SET(yday10 & 8);
				break;

			case 26:
				SET(yday10 & 4);
				break;

			case 27:
				SET(yday10 & 2);
				break;

			case 28:
				SET(yday10 & 1);
				break;

			case 29:
				cc = 2;
				break;

			case 30:
				SET(yday1 & 8);
				break;

			case 31:
				SET(yday1 & 4);
				break;

			case 32:
				SET(yday1 & 2);
				break;

			case 33:
				SET(yday1 & 1);
				break;

			case 34:
				cc = 0;
				break;

			case 35:
				cc = 0;
				break;

			case 36:
				cc = pa1;
				break;

			case 37:
				cc = pa2;
				break;

			case 38:
				cc = 0; /* SU1 */
				break;

			case 39:
				cc = 2;
				break;

			case 40:
				cc = 0; /* SU2 */
				break;

			case 41:
				SET(year10 & 8);
				break;

			case 42:
				SET(year10 & 4);
				break;

			case 43:
				SET(year10 & 2);
				break;

			case 44:
				SET(year10 & 1);
				break;

			case 45:
				SET(year1 & 8);
				break;

			case 46:
				SET(year1 & 4);
				break;

			case 47:
				SET(year1 & 2);
				break;

			case 48:
				SET(year1 & 1);
				break;

			case 49:
				cc = 2;
				break;

			case 50:
				SET(wday & 4);
				break;

			case 51:
				SET(wday & 2);
				break;

			case 52:
				SET(wday & 1);
				break;

			case 53:
				cc = 0;
				break;

			case 54:
				cc = 0;
				break;

			case 55:
				cc = 0;
				break;

			case 56:
				cc = 0;
				break;

			case 57:
				cc = 0;
				break;

			case 58:
				cc = 0;
				break;

			case 59:
				cc = 2;
				break;
			}
			result[sec] = cc;
		}
		result[60] = 2;
	}
} gen;

void setup()
{

	setCpuFrequencyMhz(80); // 80MHzで十分

	Serial.begin(115200);

	WiFi.begin(ssid, password);
	WiFi.setAutoReconnect(true);

	Serial.println();
	Serial.println();
	Serial.print("Wait for WiFi... ");
	while (!WiFi.isConnected())
	{
		Serial.printf(".");
		delay(1000);
	}
	configTzTime(tz, ntp[0], ntp[1], ntp[2]);

	if (JJY_60k_OUTPUT_PIN != -1)
	{
		ledcSetup(LEDC_60k_CHANNEL, 60000.0, LEDC_RESOLUTION_BITS);
		ledcAttachPin(JJY_60k_OUTPUT_PIN, LEDC_60k_CHANNEL);
	}

	if (JJY_40k_OUTPUT_PIN != -1)
	{
		ledcSetup(LEDC_40k_CHANNEL, 40000.0, LEDC_RESOLUTION_BITS);
		ledcAttachPin(JJY_40k_OUTPUT_PIN, LEDC_40k_CHANNEL);
	}

	if (JJY_LED_OUTPUT_PIN != -1)
		pinMode(JJY_LED_OUTPUT_PIN, OUTPUT);
}

void loop()
{
	static uint32_t min_origin_tick;
	static int last_min;
	static bool last_on_state;
	time_t t;
	t = time(&t);
	struct tm *tm = localtime(&t);
	if (last_min != tm->tm_min)
	{
		// 分の変わり目。1分ぶんのタイムコードを作成する。
		gen.year10 = (tm->tm_year / 10) % 10;
		gen.year1 = tm->tm_year % 10;
		gen.yday100 = ((tm->tm_yday + 1) / 100) % 10;
		gen.yday10 = ((tm->tm_yday + 1) / 10) % 10;
		gen.yday1 = (tm->tm_yday + 1) % 10;
		gen.yday = (tm->tm_yday + 1);
		gen.hour10 = (tm->tm_hour / 10) % 10;
		gen.hour1 = tm->tm_hour % 10;
		gen.min10 = (tm->tm_min / 10) % 10;
		gen.min1 = tm->tm_min % 10;
		gen.wday = tm->tm_wday;
		gen.mon = tm->tm_mon;
		gen.day = tm->tm_mday;
		gen.generate();
		min_origin_tick = millis();

		Serial.printf("%d%d/%d/%d (%d%d%d) %d%d:%d%d\r\n",
					  gen.year10,
					  gen.year1,
					  gen.mon + 1,
					  gen.day,
					  gen.yday100,
					  gen.yday10,
					  gen.yday1,
					  gen.hour10,
					  gen.hour1,
					  gen.min10,
					  gen.min1);
	}
	last_min = tm->tm_min;

	// 各秒ごとにお仕事
	uint32_t sub_min = millis() - min_origin_tick;
	int sec = sub_min / 1000;
	int sub_sec = sub_min % 1000;
	bool on = false;
	if (sec < 60) // sec が 60 のときがあるけどとりあえず無視
	{
		switch (gen.result[sec])
		{
		case 0:
			if (sub_sec < 800)
				on = true; // "0" コード
			break;
		case 1:
			if (sub_sec < 500)
				on = true; // "1" コード
			break;
		case 2:
			if (sub_sec < 200)
				on = true; // マーカー
			break;
		default:
			break;
		}
		if (on != last_on_state)
		{
			last_on_state = on;
			if (on)
			{
				if (JJY_60k_OUTPUT_PIN != -1)
					ledcWrite(LEDC_60k_CHANNEL, (1 << LEDC_RESOLUTION_BITS) / 2); // ディユーティー比 = 50%
				if (JJY_40k_OUTPUT_PIN != -1)
					ledcWrite(LEDC_40k_CHANNEL, (1 << LEDC_RESOLUTION_BITS) / 2); // ディユーティー比 = 50%
				if (JJY_LED_OUTPUT_PIN != -1)
					digitalWrite(JJY_LED_OUTPUT_PIN, 1);
			}
			else
			{
				if (JJY_60k_OUTPUT_PIN != -1)
					ledcWrite(LEDC_60k_CHANNEL, 0); // ディユーティー比 0 = OFF
				if (JJY_40k_OUTPUT_PIN != -1)
					ledcWrite(LEDC_40k_CHANNEL, 0); // ディユーティー比 0 = OFF
				if (JJY_LED_OUTPUT_PIN != -1)
					digitalWrite(JJY_LED_OUTPUT_PIN, 0);
			}
		}
	}
	vTaskDelay(5); // 適当な tick 分寝る
}
