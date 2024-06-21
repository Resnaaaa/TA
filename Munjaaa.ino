#include <WiFi.h>
#include <HTTPClient.h>
#include <MQUnifiedsensor.h>

// Kredensial WiFi
const char* ssid = "KONSELING";
const char* password = "";

// Alamat server
String serverName = "http://192.168.80.110/get";

// Definisi sensor MQ135
#define placa "ESP-32"
#define Voltage_Resolution 3.3
#define pin 34 // Analog input dari Arduino
#define type "MQ-135" // MQ135
#define ADC_Bit_Resolution 12 // Untuk Arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6 // RS / R0 = 3.6 ppm  

MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA); // mengatur chip WiFi sebagai station/client
  WiFi.begin(ssid, password); // koneksi ke jaringan WiFi

  while (WiFi.status() != WL_CONNECTED) { // menunggu hingga ESP32 terkoneksi ke WiFi
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi terkoneksi");
  Serial.println("Alamat IP: ");
  Serial.println(WiFi.localIP());
}

void kirim_data(float CO, float CO2, float NH4) {
  if (WiFi.status() == WL_CONNECTED) { // Cek status koneksi WiFi
    HTTPClient http;
    String url = serverName + "?CO=" + String(CO, 2) + "&CO2=" + String(CO2, 2) + "&NH4=" + String(NH4, 2);
    http.begin(url.c_str());
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("Kode Respons HTTP: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.print("Kode Error: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // Bebaskan sumber daya
  } else {
    Serial.println("WiFi Terputus");
  }
}

void setup() {
  // Inisialisasi komunikasi serial
  Serial.begin(9600);

  // Inisialisasi WiFi
  setup_wifi();

  // Inisialisasi sensor MQ135
  MQ135.setRegressionMethod(1);
  MQ135.init();
  Serial.print("Mengkalibrasi MQ-135... ");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ135.update();
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0 / 10);
  Serial.println(" selesai!");

  if (isinf(calcR0)) {
    Serial.println("Peringatan: Masalah koneksi, R0 adalah tak terhingga. Periksa kabel.");
    while (1);
  }
  if (calcR0 == 0) {
    Serial.println("Peringatan: Masalah koneksi, R0 adalah nol. Periksa kabel.");
    while (1);
  }
}

void loop() {
  // Membaca nilai sensor MQ135
  MQ135.update();
  MQ135.setA(605.18); MQ135.setB(-3.937); 
  float CO = MQ135.readSensor();
  MQ135.setA(110.47); MQ135.setB(-2.862); 
  float CO2 = MQ135.readSensor();
  MQ135.setA(102.2); MQ135.setB(-2.473); 
  float NH4 = MQ135.readSensor();

  // Menampilkan nilai sensor secara horizontal
  Serial.print("CO: "); Serial.print(CO, 2); Serial.print(" ppm | ");
  Serial.print("CO2: "); Serial.print(CO2 + 400, 2); Serial.print(" ppm | ");
  Serial.print("NH4: "); Serial.print(NH4, 2); Serial.println(" ppm");

  // Mengirim data ke server
  kirim_data(CO, CO2 + 400, NH4);

  delay(5000); // Tunggu selama 5 detik sebelum pembacaan berikutnya
}
