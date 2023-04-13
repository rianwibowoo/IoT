//library thinger esp32
#include <ThingerESP32.h>
#include <DHT.h>
#include <MQ2.h>

#include <Arduino_KNN.h>

#define USERNAME "RianW21"
#define DEVICE_ID "SVM"
#define DEVICE_CREDENTIAL "TN1nG+0#8oT352W1"

#define SSID "Rian"
#define SSID_PASSWORD "yanyan21"


ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

#define FLAME_PIN 32 //pin D1 
#define DHT_PIN 33 //pin D5 
#define DHTTYPE DHT11 
#define BUZZER_PIN 25 //pin D6 

 
//Classifier KNN 
KNNClassifier myKNN(3);

//variabel untuk kondisi deteksi api 
String kondisi,kesimpulan = "";

//variabel sensor asap 
int pin = 34;
MQ2 mq2(pin);

//variabel untuk sensor DHT
 
DHT dht(DHT_PIN, DHTTYPE);


//variabel untuk menampung temperature dan kelembaban,hasil akhir 
float sensor;
int classification;

int Sensor_Gas = 34;
int Sensor_Api = 32;
int t= 100;
float temperature,humidity;
float sensor_gas,sensor_api;

int buzzer = 25;


//konfigurasi millis, fungsi sebagai pengganti delay 
unsigned long previousMillis = 0;
const long interval = 3000; // 3 detik


void setup() { 
  pinMode(Sensor_Gas, INPUT);
  pinMode(Sensor_Api, INPUT);
  pinMode(buzzer, OUTPUT);
  Serial.begin(9600);
//Model / Data Training
float example1[] = { 26.90,72.0, 0.0};

float example2[] = { 27.10,75.0, 0.0};

float example3[] = { 39.0,80.0, 0.0};

float example4[] = { 40.0,61.0, 0.0};

float example5[] = { 23.0,59.0, 0.0};

float example6[] = { 39.70,77.0, 1.0};

float example7[] = { 37.90,79.0, 1.0};

float example8[] = { 31.0,73.0, 1.0};

float example9[] = { 32.70,67.0, 1.0};

float example10[] = { 37.0,57.0, 1.0};


//Menambahkan Model / Data Training beserta label klasifikasinya 
myKNN.addExample(example1, 0);
myKNN.addExample(example2, 0);

myKNN.addExample(example3, 0);

myKNN.addExample(example4, 0);

myKNN.addExample(example5, 0);

myKNN.addExample(example6, 1);

myKNN.addExample(example7, 1);

myKNN.addExample(example8, 1);

myKNN.addExample(example9, 1);

myKNN.addExample(example10,1);



//Input Buzzer + Flame Sensor 
pinMode(FLAME_PIN, INPUT); 
pinMode(BUZZER_PIN, OUTPUT);


//koneksi ke WiFi 
WiFi.begin(SSID, SSID_PASSWORD);
//cek koneksi WiFi

while(WiFi.status() != WL_CONNECTED)

{

// //lampu LED mati / Off 
// digitalWrite(LED_PIN, LOW); 
delay(500);
}



//apabila terkoneksi 
// digitalWrite(LED_PIN, HIGH);
//hubungkan wemos ke Thinger.IO 
thing.add_wifi(SSID, SSID_PASSWORD);

//aktifkan sensor DHT 
dht.begin();

//aktifkan sensor asap 
mq2.begin();

//data yang akan dikirim ke Thinger.IO 
thing["Dataku"] >> [](pson & out)
{
 
      out["humidity"]		= humidity; 
      out["temperature"] = temperature; 
      out["sensor_gas"]	= sensor_gas;
      out["sensor_api"]	= sensor_api;
};

}

//input sensor (flame,gas,suhu) 
void klasifikasiknn(float input[]) {

classification = myKNN.classify(input, 3); // classify input with K=5 
float confidence = myKNN.confidence();
if (classification){ 
  Serial.print("Terdeteksi Kebakaran"); kesimpulan = "Terdeteksi Kebakaran";
}

else

{

Serial.print("Tidak Terdeteksi Kebakaran"); kesimpulan = "Tidak Terdeteksi Kebakaran";
}



// print the classification and confidence 
Serial.print("\tclassification = ");
 
Serial.println(classification);



// Karena ada 2 data uji yang berdekatan dengan input dan K = 3,

// maka tingkat kepercayaanya adalah: 2/3 = ~0.67 
Serial.print("\tconfidence	= "); 
Serial.println(confidence);
}



void loop() {


//uji millis agar dapat membaca setiap 3 detik

unsigned long currentMillis = millis(); // baca waktu millis saat ini 
if(currentMillis - previousMillis >= interval)
{

//update previousMillis 
previousMillis = currentMillis;

//baca nilai sensor asap

// float* values = mq2.read(true);



//baca nilai humidity dan temperature 
humidity	= dht.readHumidity(); 
temperature = dht.readTemperature();
 


//Mengirim hasil KNN ke telegram 
if (classification > 0){ 
  thing.call_endpoint("Hasil");
}


// Mengirim pesan ke telegram bila api di deteksi

//ada api =1; tidak ada api = 0

int api = digitalRead(FLAME_PIN);


if(api == 1)

{
sensor_gas = analogRead(Sensor_Gas);//pembacaan sensor sebagai inpuanalog
Serial.print("Sensor Gas: ");
Serial.println(sensor_gas);
delay(500);
sensor_api = digitalRead(Sensor_Api);
Serial.print("Fire Class: ");
Serial.print(sensor_api);
//KNN Kebakaran Kebakaran Terdeteksi 
float parseApi = 1;
float input[] = { dht.readHumidity(), dht.readTemperature(), parseApi }; 
klasifikasiknn(input);
//ada api, nyalakan buzzer
 
digitalWrite(BUZZER_PIN, HIGH); 
kondisi = "Ada Api";

thing.call_endpoint("knnsvm");
}

else

{

//KNN Kebakaran Tidak Terdeteksi 
float parseApi = 0;
float input[] = { sensor_gas, dht.readTemperature(), parseApi }; 
klasifikasiknn(input);
//tidak ada api, buzzer mati 
digitalWrite(BUZZER_PIN, LOW); kondisi = "Aman";
}
Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(temperature);



thing.handle(); //memicu koneksi wemos ke Thinger.IO

}

}