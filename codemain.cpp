#define TOKEN "B3D94eiyrcV3TVRf7scM" // TOKEN ini kemungkinan untuk konektivitas jaringan, yang tidak didukung oleh Arduino Uno secara default.

#include <LiquidCrystal.h>
#include <DHT.h>

// Define the pins for the LCD (Pinout umum untuk Arduino Uno)
// Sesuaikan pin ini dengan koneksi fisik Anda ke Arduino Uno
#define LCD_RS 12
#define LCD_EN 11
#define LCD_D4 5
#define LCD_D5 4
#define LCD_D6 3
#define LCD_D7 2

// Initialize the LCD object
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Define the pins for the DHT sensor (Perhatikan, skema Anda menunjukkan DHT11)
// Gunakan pin digital yang tersedia pada Arduino Uno
#define DHT_PIN 7 // Contoh pin digital
#define DHT_TYPE DHT22 // Atau DHT11 jika Anda menggunakan sensor DHT11

// Initialize the DHT object
DHT dht(DHT_PIN, DHT_TYPE);

// Define the pins for the LEDs (Gunakan pin digital yang tersedia pada Arduino Uno)
#define LED_GREEN 8
#define LED_YELLOW 9
#define LED_RED 10

// Define the pin for the push button (Gunakan pin digital yang tersedia pada Arduino Uno)
#define BUTTON_PIN 6

// Define the pin for the buzzer (Gunakan pin digital yang tersedia pada Arduino Uno)
#define BUZZER_PIN A0 // Menggunakan pin analog sebagai pin digital untuk buzzer

// Define a variable to store the button state
int buttonState = 0;
int lastButtonState = 0;

// Variabel untuk toggle LCD
bool lcdDisplayOn = true;

// --- AMBANG BATAS SUHU (Anda bisa sesuaikan nilai ini) ---
const float TEMP_NORMAL_LOW = 24.0;  // Batas bawah suhu normal
const float TEMP_NORMAL_HIGH = 28.0; // Batas atas suhu normal

const float TEMP_WARN_LOW = 22.0;    // Batas bawah untuk peringatan kuning (sedikit di bawah normal)
const float TEMP_WARN_HIGH = 30.0;   // Batas atas untuk peringatan kuning (sedikit di atas normal)

// --- AMBANG BATAS KELEMBABAN (Anda bisa sesuaikan nilai ini) ---
const float HUM_NORMAL_LOW = 40.0;   // Batas bawah kelembaban normal
const float HUM_NORMAL_HIGH = 60.0;  // Batas atas kelembaban normal

const float HUM_WARN_LOW = 35.0;     // Batas bawah untuk peringatan kuning (sedikit di bawah normal)
const float HUM_WARN_HIGH = 65.0;    // Batas atas untuk peringatan kuning (sedikit di atas normal)


void setup() {
  // Initialize the serial monitor
  Serial.begin(9600);

  // Initialize the LCD
  lcd.begin(16, 2);
  lcd.print("Inisialisasi...");
  Serial.println("Inisialisasi LCD...");

  // Initialize the DHT sensor
  dht.begin();
  Serial.println("Inisialisasi DHT...");

  // Set the LED pins as outputs
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // Set the button pin as input. Gunakan INPUT_PULLUP jika tombol terhubung antara pin dan GND
  // Jika tombol terhubung antara pin dan VCC, dan ada resistor pull-down ke GND, gunakan INPUT biasa.
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Asumsi tombol terhubung ke GND
  Serial.println("Inisialisasi Pin Mode...");

  // Set the buzzer pin as output
  pinMode(BUZZER_PIN, OUTPUT);

  // Matikan semua LED dan buzzer di awal
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Beri waktu untuk DHT membaca
  delay(2000);
  lcd.clear();
}

void loop() {
  // Read the button state
  // Jika menggunakan INPUT_PULLUP, LOW berarti tombol ditekan
  buttonState = digitalRead(BUTTON_PIN);

  // Deteksi perubahan tombol untuk toggle LCD
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) { // Tombol ditekan (jika INPUT_PULLUP)
      lcdDisplayOn = !lcdDisplayOn; // Toggle state LCD
      if (!lcdDisplayOn) {
        lcd.noDisplay(); // Matikan tampilan LCD
      } else {
        lcd.display(); // Nyalakan tampilan LCD
        lcd.clear(); // Bersihkan LCD saat dihidupkan kembali
      }
    }
    delay(50); // Debounce delay
  }
  lastButtonState = buttonState;

  // Read the temperature and humidity from the DHT sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if the readings are valid
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Gagal membaca dari sensor DHT! Pastikan koneksi benar.");
    if (lcdDisplayOn) {
      lcd.setCursor(0, 0);
      lcd.print("Gagal baca DHT!");
      lcd.setCursor(0, 1);
      lcd.print("Cek Sensor!");
    }
    digitalWrite(LED_RED, HIGH); // Nyalakan LED merah sebagai indikator error
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(BUZZER_PIN, LOW); // Jangan bunyikan buzzer saat error sensor
    delay(2000); // Tunggu lebih lama jika ada error
    return; // Keluar dari loop dan coba lagi
  }

  // Print the readings to the serial monitor
  Serial.print("Suhu: ");
  Serial.print(temperature);
  Serial.println(" *C");
  Serial.print("Kelembaban: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Status Tombol (0=tekan, 1=lepas): ");
  Serial.println(buttonState);
  Serial.print("LCD Aktif: ");
  Serial.println(lcdDisplayOn ? "Ya" : "Tidak");


  // Display the readings on the LCD if it is on
  if (lcdDisplayOn) {
    lcd.setCursor(0, 0);
    lcd.print("Suhu: ");
    lcd.print(temperature);
    lcd.print((char)223); // Karakter derajat
    lcd.print("C   "); // Spasi untuk membersihkan sisa karakter
    lcd.setCursor(0, 1);
    lcd.print("Kelembaban: ");
    lcd.print(humidity);
    lcd.print("%   "); // Spasi untuk membersihkan sisa karakter
  }

  // --- LOGIKA KONTROL LED DAN BUZZER (Mempertimbangkan Suhu dan Kelembaban) ---
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Cek kondisi "Jauh di luar batas" (Merah + Buzzer) untuk Suhu ATAU Kelembaban
  if (temperature < TEMP_WARN_LOW || temperature > TEMP_WARN_HIGH ||
      humidity < HUM_WARN_LOW || humidity > HUM_WARN_HIGH) {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(BUZZER_PIN, HIGH); // Buzzer menyala bersamaan dengan LED merah
    Serial.println("Status: Jauh di Luar Batas (Merah + Buzzer)");
  }
  // Cek kondisi "Sedikit di luar batas" (Kuning) untuk Suhu ATAU Kelembaban
  // Kondisi ini hanya dieksekusi jika tidak masuk ke kondisi "Merah" di atas
  else if ((temperature < TEMP_NORMAL_LOW && temperature >= TEMP_WARN_LOW) ||
           (temperature > TEMP_NORMAL_HIGH && temperature <= TEMP_WARN_HIGH) ||
           (humidity < HUM_NORMAL_LOW && humidity >= HUM_WARN_LOW) ||
           (humidity > HUM_NORMAL_HIGH && humidity <= HUM_WARN_HIGH)) {
    digitalWrite(LED_YELLOW, HIGH);
    Serial.println("Status: Sedikit di Luar Batas (Kuning)");
  }
  // Cek kondisi "Normal" (Hijau) untuk Suhu DAN Kelembaban
  // Kondisi ini hanya dieksekusi jika tidak masuk ke kondisi "Merah" atau "Kuning"
  else if ((temperature >= TEMP_NORMAL_LOW && temperature <= TEMP_NORMAL_HIGH) &&
           (humidity >= HUM_NORMAL_LOW && humidity <= HUM_NORMAL_HIGH)) {
    digitalWrite(LED_GREEN, HIGH);
    Serial.println("Status: Suhu dan Kelembaban Normal (Hijau)");
  }
  // Kasus lain (misal, jika ada kondisi yang tidak terdefinisi secara spesifik)
  // Untuk memastikan semua LED mati jika tidak ada kondisi yang terpenuhi
  else {
      // Tidak ada LED yang menyala jika tidak ada kondisi di atas yang terpenuhi
      Serial.println("Status: Kondisi tidak terklasifikasi, semua LED mati.");
  }


  // Wait for 2 seconds before the next reading (Anda bisa sesuaikan delay ini)
  delay(2000);
}
