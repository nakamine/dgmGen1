#define getNbitFromNum(num,n) ((num&(0x01 << n))>0 ? 0x01 : 0x00)

// K155IDの制御ピン
const int output[] = { 13, 11, 10, 12 };
// 入力ボタンの制御ピン
const int button1 = 19;
const int button2 = 18;
const int button3 = 17;
const int button4 = 16;
// ニキシー管の制御ピン（フォトダイオードのON/OFF）
const int tube[8] = { 9, 8, 7, 6, 5, 4, 3, 2};
// ニキシー管の左右のドットの制御ピン（フォトダイオードのON/OFF）
const int lPiriod = 15;
const int rPiriod = 14;
// ボタンの状態を保存する変数
int btnSts1 = 0;
int btnSts2 = 0;
int btnSts3 = 0;
int btnSts4 = 0;
// ダーバージェンスを設定する項目（10は非表示
int displayNum[10][8] = {
{0,10,0,0,0,0,0,0}, // 1
{0,10,5,7,1,0,2,4}, // 2
{0,10,5,7,1,0,1,5}, // 3
{0,10,3,3,4,5,8,1}, // 4
{1,10,1,3,0,4,2,6}, // 5
{1,10,1,3,0,2,0,5}, // 6
{1,10,1,3,0,2,1,2}, // 7
{1,10,1,3,0,2,3,8}, // 8
{1,10,0,4,8,5,9,6}, // 9
{3,10,4,0,6,2,8,8} // 10
};
// ダイバージェンスのドットの表示位置を設定する項目
// 1が右のドット、2が左ドット、3が左右両方のドット
int displayDot[10][8] = {
{0,1,0,0,0,0,0,0}, // 1
{0,1,0,0,0,0,0,0}, // 2
{0,1,0,0,0,0,0,0}, // 3
{0,1,0,0,0,0,0,0}, // 4
{0,1,0,0,0,0,0,0}, // 5
{0,1,0,0,0,0,0,0}, // 6
{0,1,0,0,0,0,0,0}, // 7
{0,1,0,0,0,0,0,0}, // 8
{0,1,0,0,0,0,0,0}, // 9
{0,1,0,0,0,0,0,0}  // 10
};

// 現在選択されているダイバージェンスの番号（displayNumのインデックス）
int selectNo = 0;
// 現在表示されているDGメータの値
int nowDisplayData[8]    = {0,0,0,0,0,0,0,0};
// 現在表示されているDGメータのドット
int nowDisplayDotData[8] = {0,0,0,0,0,0,0,0};

// 現在の表示モード（初期値はDGメータ
int mode = 0;
// 時計の累積時間（秒）
int time = 0;
// 時計の初期値
int hh = 9;
int mm = 26;  
int ss = 5;
// 現在の時計表示の値
int clockTime[8] = {0,0,0,0,0,0,0,0};

// 固定値
// ドット無し
int noDot[8]   = {0,0,0,0,0,0,0,0};
// 時計用のドット表示
int timeDot[8] = {0,0,1,0,0,1,0,0};


// 起動して最初に一度実行されるところ
void setup() {                
  // ArduinoのIOピンを用途に合わせて初期化する
  for ( int i=0; i<4; ++i ) {
    pinMode(output[i], OUTPUT);
  }
  
  for ( int i=0; i<8; ++i ) {
    pinMode(tube[i], OUTPUT);
  }
  
  pinMode(lPiriod, OUTPUT);
  pinMode(rPiriod, OUTPUT);
  
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
  
  // 最初に表示されるDGメータの値をセット
  dgSet();
}

// ここが繰り返し実行されるところ
void loop() {
  // 時計表示（仮）のカウントアップ。このやり方（millis）だと50日間で時計がおかしくなる。
  // 時計としてちゃんと使うのにはRTCを使って実装が必要。
  int tmpTime = time;
  time = millis() / 1000;
  if (tmpTime != time) { 
    ss++;
    if (ss >= 60) {
      ss = 0;
      mm++;
      if (mm >= 60) {
        mm = 0;
        hh++;
        if (hh >= 24) {
          hh = 0;
          mm = 0;
          ss = 0;
        }
      }
    }
  }
  
  // ボタンの状態取得
  btnSts1 = digitalRead(button1);
  btnSts2 = digitalRead(button2);
  btnSts3 = digitalRead(button3);
  btnSts4 = digitalRead(button4);

  if (btnSts1 == LOW) {
    // モード切替ボタン
    delay(500);
    // mode 0 DG
    // mode 1 Clock
    if (mode == 1) {
      mode = 0;
    } else {
      mode = 1;
    }
    //changeDemo();
    
  } else if (btnSts2 == LOW) {
    // upボタン
    delay(500);
    if (mode == 0) {
      dgUp(); // DGを次の値にする
    }
  } else if (btnSts3 == LOW) {
    // downボタン
    delay(500);
    if (mode == 0) {
      dgDown(); // DGを前の値にする
    }
  } else if (btnSts4 == LOW) {
    // 編集モードボタン
    delay(500);
    // 未実装。時刻合わせとかオリジナルダイバージェンスとかの編集モードに入るボタンの予定
  }
  
  // 表示モードによって表示を変える
  if (mode == 0) {
    // DG表示状態
    dispData(nowDisplayData, nowDisplayDotData);
  } else if (mode == 1) {
    timeGet(); // clockTimeを今の時間に更新
    dispData(clockTime, timeDot);
  }
}


////////////////////
// ここから下は関数
////////////////////

// clockTimeを更新
void timeGet() {
  clockTime[0] = (hh / 10) % 10;
  clockTime[1] = hh % 10;
  clockTime[2] = 10;
  clockTime[3] = (mm / 10) % 10;
  clockTime[4] = mm % 10;
  clockTime[5] = 10;
  clockTime[6] = (ss / 10) % 10;
  clockTime[7] = ss % 10;
  
}

// DGメータ値を次の値にする
void dgUp() {
  selectNo++;
  if (selectNo > 9) {
    selectNo = 0;
  }
  changeDemo();
  dgSet();
}

// DGメータ値を前の値にする
void dgDown() {
  selectNo--;
  if (selectNo < 0) {
    selectNo = 9;
  }
  changeDemo();
  dgSet();
}

// DGメータの値を表示用の変数に設定
void dgSet() {
  for(int i = 0; i < 8; i++) { 
    nowDisplayData[i]    = displayNum[selectNo][i];
    nowDisplayDotData[i] = displayDot[selectNo][i];
  }
}

// DGメータの値を変更する際のランダム表示エフェクト
void changeDemo() {
  for (int i = 0; i < 25; i++) { // 繰り返し
    int tmp[8];
    for (int j = 0; j < 8; j++) { // 桁数
      tmp[j] = random(0,10);
    }
    for (int k = 0; k < 6; k++) { // 同じ値の表示時間
      dispData(tmp , noDot);
    }
  }
}

// DGメータに値の表示を実行
void dispData(int* dataVal, int* dotVal) {
  // ニキシー管を左から切り替えてダイナミック点灯させる
  for(int i = 0; i < 8;i++) {
    // K155IDに適切な値を送りニキシー管の任意の数字を選択
    for ( int j=3; j<=0; --j ) {
      digitalWrite(output[j], getNbitFromNum(dataVal[i],j) );
    }
    // ニキシー管の電源をON
    digitalWrite(tube[i], HIGH);
    // 500マイクロ秒点灯して表示
    delayMicroseconds(500);
    // ニキシー管の電源をOFF 
    digitalWrite(tube[i], LOW);
    // 200マイクロ秒消灯。待たないと他のニキシー管に残像がでるため。
    delayMicroseconds(200); 
    // ドットの表示があればドットの表示をする。
    if (dotVal[i] == 1) {
      // K155IDに数字を表示しないような情報をおくる。
      for ( int j=0; j<4; ++j ) {
        digitalWrite(output[j], 1);
      }
      digitalWrite(rPiriod, HIGH);
      digitalWrite(tube[i], HIGH);
      delayMicroseconds(200); // ドットは同じ時間表示させると明るすぎるため時間を短く設定
      digitalWrite(tube[i], LOW);   
      digitalWrite(rPiriod, LOW);      
      delayMicroseconds(200);
    } else if (dotVal[i] == 2) {
      for ( int j=0; j<4; ++j ) {
        digitalWrite(output[j], 1);
      }
      digitalWrite(lPiriod, HIGH);
      digitalWrite(tube[i], HIGH);
      delayMicroseconds(200);
      digitalWrite(tube[i], LOW);   
      digitalWrite(lPiriod, LOW);      
      delayMicroseconds(200);
    } else if (dotVal[i] == 3) {
      for ( int j=0; j<4; ++j ) {
        digitalWrite(output[j], 1);
      }
      digitalWrite(rPiriod, HIGH);
      digitalWrite(tube[i], HIGH);
      delayMicroseconds(200);
      digitalWrite(tube[i], LOW);   
      digitalWrite(rPiriod, LOW);      
      delayMicroseconds(200);
      digitalWrite(lPiriod, HIGH);
      digitalWrite(tube[i], HIGH);
      delayMicroseconds(200);
      digitalWrite(tube[i], LOW);   
      digitalWrite(lPiriod, LOW);      
      delayMicroseconds(200);
    } 
    
  }

}
