#include <KashiwaGeeks.h>

ADB922S LoRa;  // LoRaWANシールドのインスタンスを生成

//================================
//       デバイス初期化関数
//================================
#define BPS_9600     9600
#define BPS_19200   19200
#define BPS_57600   57600
#define BPS_115200 115200

void start()
{
    /*  Setup console */

    ConsoleBegin(BPS_57600);

    //DisableConsole();
    //DisableDebug();

    ConsolePrint(F("**** Step5 Starts*****\n"));

    /*  LoRaWANシールドのインスタンスを初期化  */
    if ( LoRa.begin(BPS_19200) == false )
    {
        for(;;)   // 初期化できなければABORT
        {
            LedOn();
            delay(300);
            LedOff();
            delay(300);
        }
    }

    /*  LoRaWANにJOIN  */
    LoRa.reconnect();
    LoRa.setDr(3);

    /*  WDTのインターバル設定 1, 2, 4 or 8 秒のいずれか  */
    //setWDT(1);    
}

//================================
//   定期的に実行される関数
//================================
void task1(void)
{
 while(true){
   sendTemp();            // 　LoRaWAN送信処理
   delay(5000);
 }
}

/*-------------------------------------------------------------*/
void task2(void)
{

}


//===============================
//            Execution interval
//    TASK( function, interval by second )
//===============================
TASK_LIST = {
        TASK(task1, 0, 1),
        //TASK(task2, 2, 10),
        END_OF_TASK_LIST
};

/*-------------------------------------------------------------*/

#define LoRa_fPort_TEMP  12        // port 12 = 温度湿度気圧等

//
// 温度湿度気圧を LoRa送信する
//
void sendTemp()
{
  short port = 100;    // port 12 = Temp

  unsigned long temp    = random(139000000);
  unsigned long humi    = random(340000000);
  unsigned long press   = 0x00000003;

  // LoRaWANでデータを送信する
  //LoRa.sendString(port, true, F("%04x%04x%06lx"), temp, humi, press);
  LoRa.sendString(port, true, F("%08x%08x%08x"), temp, humi, press);
}

/*   End of Program  */

