#include<Wire.h>
#include<Adafruit_GFX.h>
#include<Adafruit_SSD1306.h>

// ディスプレイ変数の宣言
Adafruit_SSD1306 display(-1);

#define beep 9  //圧電ブザー用のピン

#define echo1 2 // Echo Pin（面）
#define trig1 3 // Trigger Pin
#define echo2 4 // Echo Pin（線）
#define trig2 5 // Trigger Pin
#define echo3 6 // Echo Pin（点）
#define trig3 7 // Trigger Pin

#define a 0.5 //LPFの係数。0.8だとちょっと遅い。即応性
#define b 3 //オフセット

double sDistance1, sDistance2, sDistance3; //LPF用に前回の距離を保持しておく
double Duration = 0; //受信した間隔
double distance, Distance = 0; //距離
int dist, dist3, dist2, dist1, ssurf_t, sline_t, spoint_t, point_t, line_t, surf_t, sec, minute;  //測定距離と算出時間と設定時間
int mode = 0;

void setup() {
  //Serial.begin( 9600 );
  pinMode(echo1, INPUT);
  pinMode(trig1, OUTPUT);
  pinMode(echo2, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo3, INPUT);
  pinMode(trig3, OUTPUT);
  pinMode(beep, OUTPUT);

  TIMSK0 = 0; //タイマーなのに、タイマー管理は使わない

  // ディスプレイの初期化
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

void loop(){
  if(mode == 0){
    // ディスプレイをクリア
    display.clearDisplay();
    // 出力する文字の大きさ
    display.setTextSize(3);
    // 出力する文字の色
    display.setTextColor(WHITE);
    // 文字の一番端の位置
    display.setCursor(0, 0);
    display.print("SEPATIM");//表示もリセット
    display.display();
    //delay(1000);
    oled_clear();

    dist3 = us(echo3, trig3);
    dist2 = us(echo2, trig2);
    dist1 = us(echo1, trig1);

    if(dist1 <= 9 || dist2 <= 12 || dist3 <= 6){
      mode = 1; //リセット中でも、近づけたらリセットできるように。より直感的な操作を
      tone(beep, 440, 100); //tone(pin, 周波数, 長さ);
      delay(100);//少し音がなった後にモード切り替え
    }
  }
  //近づけたら自動的にスタートする
  else if(mode == 1){
    //10秒単位
    dist3 = us(6, 7);
    if(dist3 <= 5 && dist3 > 0){
      point_t = dist3 * 10;
    }else if(dist3 < 9 && dist3 > 5){
      point_t = 45;
    }else if(dist3 >= 9){  //else{point_t = 0;}
      point_t = 0;
    }

    //1分単位
    dist2 = us(4, 5);
    if(dist2 <= 9 && dist2 > 0){
      line_t = dist2;
    }else if(dist2 < 12 && dist2 > 9){
      line_t = 9;
    }else if(dist2 >= 12){
      line_t = 0;
    }

    //10分単位
    dist1 = us(2, 3);
    if (dist1 <= 6 && dist1 > 0){
      surf_t = dist1 * 10;
    }else if(dist1 < 9 && dist1 > 6){
      surf_t = 60;
    }else if(dist1 >= 9){
      surf_t = 0;
    }

    if(dist1 >= 9 && dist2 >= 12 && dist3 >= 6){
      mode = 0; //カウントダウン中でも、遠くまで離したらリセットできるように。より直感的な操作を
      tone(beep, 440, 100); //tone(pin, 周波数, 長さ);
      delay(100);//少し音がなった後にモード切り替え
    }

    //距離が変化しない場合はカウントダウン
    if(ssurf_t == surf_t && sline_t == line_t && spoint_t == point_t){
      delay(190000); //1秒ごとに。200000じゃちょっと遅いことがわかった。
      sec = sec - 1; //カウントダウン
      //delay(1000);  //ここでdelay()を入れてみようかな→ダメだった

      //繰り下がり
      if(sec < 0){
        minute = minute - 1;
        sec = 59;
      }
      //タイムアップでブザーがなる。
      if(minute <= 0 && sec == 0){
      mode = 2;
      }
    }
    //距離が変わったらその都度設定時間に反映
    else{
      sec = point_t;
      minute = line_t + surf_t;
      if(minute == 0 && sec == 0){
          mode = 0;
      }
    }

    //ディスプレイ表示
    //ディスプレイをクリアする処理を関数化したい。
    oled_clear();
    //10分以下なら一つ開ける
    if(minute == 0){
      display.print(" 0:");
    }
    else if(minute < 10){
      display.print(" ");
      display.print(minute);
      display.print(":");
    }else{
      display.print(minute);
      display.print(":");
    }
    //0秒のときだけは”00”に
    if(sec == 0){
      display.println("00");
    }if(sec < 10){
      display.print("0");
      display.println(sec);
    }else{
      display.println(sec);
    }
    //バッファした内容を出力
    display.display();

    ssurf_t = surf_t;
    sline_t = line_t;
    spoint_t = point_t;

  }
  //タイムアップ
  else if(mode == 2){
    oled_clear();
    display.println("beep!");
    display.display();
    tone(beep, 440, 500);
    delay(500);

    dist3 = us(echo3, trig3);
    dist2 = us(echo2, trig2);
    dist1 = us(echo1, trig1);

    if(dist1 >= 9 && dist2 >= 12 && dist3 >= 6){
      mode = 0; //カウントダウン中でも、遠くまで離したらリセットできるように。より直感的な操作を
      tone(beep, 440, 100); //tone(pin, 周波数, 長さ);
      delay(100);//少し音がなった後にモード切り替え
    }
    //noTone(beep);
  }
}

//距離計測を関数化
int us(int echo, int trig){

  digitalWrite(trig, LOW);
  delayMicroseconds(10);
  digitalWrite(trig, HIGH); //超音波を出力
  delayMicroseconds(10); //
  digitalWrite(trig, LOW);
  Duration = pulseIn(echo, HIGH); //センサからの入力

  if (Duration > 0) {
    Duration = Duration/2; //往復距離を半分にする
    distance = Duration*340/10000; // 音速を340m/sに設定
    if(distance <= 30.0 && distance >= 0){
      if(trig == trig1){
        Distance = a * sDistance1 + (1-a) * distance;  //LPF
        sDistance1 = Distance;
      }else if(trig == trig2){
        Distance = a * sDistance2 + (1-a) * distance;  //LPF
        sDistance2 = Distance;
      }else if(trig == trig3){
        Distance = a * sDistance3 + (1-a) * distance;  //LPF
        sDistance3 = Distance;
      }
      dist = Distance - b;
      return dist;
      //delay(1000);
    }
  }else{
    if(trig == trig1){
      return round(sDistance1);  //変数sDistanceはdouble型のため、丸める。
    }else if(trig == trig2){
      return round(sDistance2);  //変数sDistanceはdouble型のため、丸める。
    }else if(trig == trig3){
      return round(sDistance3);  //変数sDistanceはdouble型のため、丸める。
    }
    //delay(10);
  }
  //return dist; //最も近い整数値に丸めるべくint型にキャストした後出力
}


//ディスプレイの初期化用
void oled_clear(){
  // ディスプレイをクリア
  display.clearDisplay();
  // 出力する文字の大きさ
  display.setTextSize(4);
  // 出力する文字の色
  display.setTextColor(WHITE);
  // 文字の一番端の位置
  display.setCursor(0, 0);
}
