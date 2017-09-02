// tam0509 Edits 05082009 Changed Ozone settings to target instead of tracking and hours of 1000 - 1800
// tam0509 Modified from SOYFACE050307.c

// tam0520 Modified for no high O3 output shutdown.
// crm0609 edited 050809 - changed ozone hours to 0900 - 1600 to account for Central Standard Time
// crm0709 edited 061509 - changed ozone hours to 0900 - 1800 to allow for 2 more hours of fumigation
// kkp 2013-5-22 enabled networking, removed unused menu items, corrected sector numbering in calibrate

// cmm0517 annotated define parameters for easy user inputs. Go to line 722 to change

#use SUNRISE.lib
//#define NETWORK  				// Comment Out to not load Network Drivers
#define INITIALIZE_PARAMS  //  ****MUST HAVE THIS enabled otherwise will use defaults**** if screen avail then == Run the first time to config settings - Comment out after that
#ifdef NETWORK
//	#define IPDOWNLOAD 			// Comment out to remove Network Firmware Upgrade
#endif
#define Polled  0				// Set 0 to dump data per second/minute, 1 to wait for command
#define OZONE_MULT 1    	// Leave set to 1, now set through host computer
#define ADBRD 1  				// Slot # for Analog Input Board
#define DABRD 2      		// ********Slot # for Analog CURRENT (mA) Output Board
#define RELAY 4   			// Slot # for Relay Output Board
#define DIGIO 6    			// Slot # for Dig IO Board
#define INP_Dew 7				// A/D Channel for Dewpt input
#define INP_WSpd 8   		// A/D Channel for Wind Speed Input
#define INP_WDir 9  			// A/D Channel for Wind Direction Input
#define INP_WDiff 10 		// A/D CHannel for Wind Ground (Differential Measure)
#define GRAB_TIME 4 			// Frequency for Data Transmit to Serial/IP (seconds)
#define OZONE_MAX 200 		// Maximum value Ozone can set to
#define delay_time 120		// Time Delay for Ozone Restart after low wind speed shut down
#define N_CHANNELS 1			// Number of Channels to Operate (Layers)

//  *** MOST USER OPTIONS ABOVE THIS LINE  - Network Configure listed Below ***
//  *** Also change calc_day routine to include correct lon, lat, and time zone for correct sunrise/set

#ifdef NETWORK
#define TCPCONFIG 0
#define _PRIMARY_STATIC_IP		"192.168.1.209"
#define _PRIMARY_NETMASK		"255.255.255.0"
#define MY_NAMESERVER		"192.168.1.1"
#define MY_GATEWAY			"192.168.1.1"
#define USE_ETHERNET		1
#define IFCONFIG_ETH0 \
				IFS_IPADDR,aton(_PRIMARY_STATIC_IP), \
				IFS_NETMASK,aton(_PRIMARY_NETMASK), \
				IFS_UP
#define UDP_SOCKETS 6	// allow enough for downloader and DHCP
#define MAX_UDP_SOCKET_BUFFERS 6
#define LOCAL_PORT   425
#define REMOTE_IP       "255.255.255.255" /*broadcast*/
#define  DEST       "192.168.1.155"
#define  PORT     1001
#define  ETH_MTU      900
#define  BUFF_SIZE    (ETH_MTU-40)  //must be smaller than (ETH_MTU - (IP Datagram + TCP Segment))
#define  TCP_BUF_SIZE ((ETH_MTU-40)*4) // sets up (ETH_MTU-40)*2 bytes for Tx & Rx buffers
#define MAX_TCP_SOCKET_BUFFERS 6
#define ETH_MAXBUFS  2


#use "dcrtcp.lib"
#ifdef IPDOWNLOAD
#define UDPDL_LOADER "E:/Z-World/UDPDL/Loaders/pdl-generic-D.bin"
#use "udpdownl.lib"
#endif

#define INCOMING_IP     0        //except all connections
#define INCOMING_PORT   0        //except all ports
#define TIME_OUT        15    // if in any state for more than a 2 seconds re-initialize

#endif

#memmap xmem




#define DELAYTIME 50 //350



//---------------------------------------------------------
// Macro's
//---------------------------------------------------------
#define MAXDISPLAYROWS	4
#define LEDOFF				0
#define TOGGLE				1
#define INCREMENT			2
#define OPERATE			3

#define ASCII				0
#define NUMBER				1

#ifdef NETWORK
tcp_Socket sock_A, sock_B, sock_C;
int bytes_A, bytes_B, bytes_C;
int state_A, state_B, state_C;
int sent_A, sent_B, sent_C;
int rcvd_A, rcvd_B, rcvd_C;
long statetime_A, statetime_B, statetime_C;
static char buff_A[BUFF_SIZE], buff_B[BUFF_SIZE], buff_C[BUFF_SIZE];
word my_port_A, my_port_B, my_port_C;
char str_A[50];
char str_B[50];
char str_C[50];
char* strscan;
#endif

long settings[10];
void* save_data[1];
unsigned int save_lens[1];

//----------------------------------------------------------
// Main_Menu options
//----------------------------------------------------------
const char *main_menu[] = {
    " <<<<Main Menu>>>>",
    "1)Return to Program",
    "2)Turn Ozone ON",
    "3)Turn Ozone OFF",
    "4)CaliBrate Unit",
    "5)Set Date & Time",
    "6)Config Settings",
    "7) *** RESET *** ",
    //"8) Set IP Address",
    NULL
};
const char *treatments[] = {
    "CO2:",
    "O3 :",
    "OFF:",
    NULL
};
const char *anemometer[ ]= {
    "RMYoung12005",
    "Sonic",
    NULL
};

const char *boolean[ ]= {
    "False",
    "True",
    NULL
};
const char *display[ ]= {
    "Single",
    "Double",
    NULL
};


const char *token;
const char *delim = " ,:/";



//----------------------------------------------------------
// Structures, arrays, variables
//----------------------------------------------------------
fontInfo fi6x8, fi8x10, fi12x16;
windowFrame textWindow;

typedef struct  {
    int data;
    char *ptr;
} fieldupdate;

struct tm CurTime;

char szTime[40];
char szString[20];
const char Days[] = {"SunMonTueWedThuFriSat"};
const char Months[] = {"JanFebMarAprMayJunJulAugSepOctNovDec"};


int UNIT_ID;
int nighttime_local;
int ledCntrl;
int beeperTick, timerTick ;
int max_menu_options;
int max_cmds_options;
unsigned long ulTime;
char *keybuffer;






#define CINBUFSIZE 127
#define COUTBUFSIZE 127
#define DINBUFSIZE 127
#define DOUTBUFSIZE 127
#define MAX_SENTENCE 100


#define SETRAW 16				//offset to get raw analog data
#define INS 1
#define OUTS 0
#define BANK_A INS		//bank A inputs
#define BANK_B OUTS		//bank B outputs
#define TURNON 1
#define TURNOFF 0


#define DIM_S           10              //
#define DIM_L           45              //

#define PURGE_TIME	120

typedef unsigned char uchar;
typedef unsigned int  uint;
//typedef unsigned long ulong;

static unsigned long store;				// physical memory address to write to



//---------------------------------------------------------------------------
// Definitions of variable
//---------------------------------------------------------------------------
uchar wI;
uchar byIChA[N_CHANNELS+1];
uchar byIChB[N_CHANNELS+1];
uint outd;

uint scan;
uint RECORD;                            //
uint nCont01[N_CHANNELS+1];             //
uint nCont02[N_CHANNELS+1];             //
uint nCont03[N_CHANNELS+1];             //
uint nCont04[N_CHANNELS+1];             //
uint nContao[N_CHANNELS+1];             //
uint LAYER[N_CHANNELS+1];
uint LAYERRELAY[N_CHANNELS+1];
uint sec_count;
uint nSel;                              //
uint chn;
uint byICh;
uint nI;
uint cnt45;
uint cnt10;
uint bySect;
uint opSect[N_CHANNELS+1];
uint MOBILE;
uint MobDir[9];
uchar T1;                                //
uchar T2;                                //
uchar T3;                                //
uchar T4;                                //
uchar T5;                                //
uchar T6;                                //
uint V1;                                //
uint V2;                                //
uint V3;                                //
uint V4;                                //
uint V5;                                //
int sector;
uint i;
char menudisp;
float fConc[N_CHANNELS+1];
float fDew;
float fTemp;
float fRh;
float fFpro0[N_CHANNELS+1] [DIM_L+1];     //
float fFdif0[N_CHANNELS+1] [DIM_L+1];     //
float fFwin[DIM_L+1];                   //
float fFspro[N_CHANNELS+1];             //
float fFsdif[N_CHANNELS+1];             //
float fFswin[N_CHANNELS+1];             //
float fF10win[DIM_L+1];                 //
float fF10[N_CHANNELS+1];               //
float fVp[N_CHANNELS+1];                //
float fV0[N_CHANNELS+1];                //
float fint0[N_CHANNELS+1];              //
float fVs[N_CHANNELS+1];					//
float fIrgaA;                           //
float fIrgaB;                           //
float fMedIrgaB;                        //
float fFwind;                           //
float fFlow;                            //
float MobSum;                           //
float MobCorr;                          //
float Direz;                            //
float fResA;                            //
float fResB;                            //
float Sumw;                             //
float Sumd;                             //
float f45;
float f10;
float s45;
float s10;

uint giorno;
uint mese;
uint ggiul;
float dayl;
float starttime_ozone = 9;
float endtime_ozone = 17;
float starttime_co2 = 8;
float endtime_co2 = 17;
float sunr;
float suns;
float ora;  //Current Decimal Time

float fwind;
float fval[N_CHANNELS+1];
float fmpro[N_CHANNELS+1];
float fmdif[N_CHANNELS+1];
float fmwin[N_CHANNELS+1];
float fm10w[N_CHANNELS+1];
float feint[N_CHANNELS+1];
float fepro[N_CHANNELS+1];
float fedif[N_CHANNELS+1];
float fcor[N_CHANNELS+1];
float fpps[N_CHANNELS+1];
float TARG[N_CHANNELS+1];
float nighttimeCO2;
float AD_GAIN[N_CHANNELS+1];
float AD_OFFSET[N_CHANNELS+1];
float DA_MULT[N_CHANNELS+1];
float Wind_Mult;
float Wind_Offset;

float FAINT[N_CHANNELS+1];
float FAPRO[N_CHANNELS+1];
float FADIF[N_CHANNELS+1];
float FCW[N_CHANNELS+1];
float ENDCONO;
float fRet;
float V_OUT_MIN;
float V_OUT_MAX;


float MB;
float fTxlm;
float fMedo;
float fVento;

float medo[N_CHANNELS+1];
float vento[N_CHANNELS+1];
float txlm[N_CHANNELS+1];

uint memPointer;

char ozonator_loc;
char ozonator_rem;
char CO2_loc;
char CO2_rem;

char flow;
char purge;

char delay_restart;  // Ozone Restart Timer
unsigned long delay_restart_time;






char inkey;
char nighttime_remote;
int purge_off;
char DA_Channel[8];
char locBuf[50];
char SENTENCE_D[MAX_SENTENCE];
char SENTENCE_C[MAX_SENTENCE];
char SENTENCE_IP[MAX_SENTENCE];
int input_char;
int string_pos_c;
int string_pos_d;
int string_pos_ip;
int backlight;
int timeOn;
const int timeDelay = 10;
int DISPLAY; //Set Display Options

int StatusCode;

// Sunrise Calculations
const double pi = 3.14159265358979;
double degs;
double rads;
double L,g,daylen;
double twam,altmax,noont,settm,riset,twpm;
const double AirRefr = 34.0/60.0; // athmospheric refraction degrees
// Sunrise Calculations


struct tm		rtc;		//Time and Date struct
#ifdef NETWORK
udp_Socket sock;    //Added 121405
tcp_Socket socket;
#endif
char message[250];


void calc_day(void);
void  fControl(void);
void readString_C(void);
void readString_D(void);
void readSerialC(void);
void readSerialD(void);
void calibrate(int sector);
void cont_loop(void);
void sector_select(void);
void wind_avg(void);
void minute_average(void);
void second_data(void);
void set_output(void);
void menu(void);
void keypress(void);
void SetDateTime(void);
void SetConfig(void);
void DispMenu(void);
void Reset_Ozone_Vars(void);
void readString_IP(void);
#ifdef NETWORK
int receive_packet_udp(void)
{
    auto int i;
    static char buf[128];

#GLOBAL_INIT
    {
        memset(buf, 0, sizeof(buf));
    }

    /* receive the packet */
    if (-1 == udp_recv(&sock, buf, sizeof(buf))) {
        /* no packet read. return */
        return 0;
    }


    for(i=0; i<sizeof(buf); i++) {
        input_char = buf[i];
        if(input_char == '\r') {
            //printf("Received Return \n");
            SENTENCE_IP[string_pos_ip++] = '\0';
            readString_IP();
        } else if(input_char > 0) {
            SENTENCE_IP[string_pos_ip] = input_char;
            string_pos_ip++;
            //printf("%d \t %c \n",input_char,input_char);
        }
    }




    memset(buf, 0, sizeof(buf));

    return 1;
}

int receive_packet(tcp_Socket* sock, char *buff, int* bytes, int* rcvd, char* str)
{
    /* receive the packet */
    *bytes = sock_fastread(sock,buff,BUFF_SIZE);

    switch(*bytes) {
    case -1:
        return 4; // there was an error go to state 4 (NO_WAIT_CLOSE)
    case  0:
        return 2; // connection is okay, but no data received
    default:
        (*rcvd)++;
        //*strcat(buff, *input);


        buff[*bytes] = '\0';
        strcat(str,buff);
        strscan = strrchr(str, '\r');
        if(strscan !='\0') {
            //printf("%s, %d \n", str, strlen(str));
            strcpy(SENTENCE_IP, str);
            readString_IP();
            str[0] = '\0';
        }



        return 3;	//now go to state 3 (SEND)
    }
}

int send_packet(tcp_Socket* sock, char* buff, int* bytes, int* sent)
{
    return 2;
    /* send the packet
    *bytes = sock_fastwrite(sock,buff,*bytes);
    switch(*bytes)
    {
    	case -1:
    		return 4; // there was an error go to state 4 (NO_WAIT_CLOSE)
    	default:
    		(*sent)++;
    		return 2;	//now go to state 2 (RECEIVE)
    }                 */
}

MyHandle(tcp_Socket* sock, char* buff, int* bytes, int* state, word my_port,
         long* statetime, int *sent, int *rcvd, char* str)
{
    tcp_tick(sock);
    switch(*state) {
    case 0:/*INITIALIZATION*/										// listen for incoming connection
        tcp_listen(sock,my_port,INCOMING_IP,INCOMING_PORT,NULL,0);
        (*statetime) = SEC_TIMER+TIME_OUT;						// reset the statetime
        *sent = *rcvd = 0;											// reset num of packets send and rcvd
        (*state)++;														// init complete move onto next state
        printf("%02d:%02d:%02d, \t Port Initialized\n",rtc.tm_hour,rtc.tm_min,rtc.tm_sec);
        break;

    case 1:/*LISTEN*/
        if(sock_established(sock)) {								// check for a connection
            (*state)++;    //   we have connection so move on
        } else if ((long)(SEC_TIMER-(*statetime)) > 0) {		// if 1 sec and no sock
            *state = 4;    //	  abort and re-init
        }

        break;
    case 2:/*RECEIVE*/
        *state = receive_packet(sock, buff, bytes, rcvd, str);	// see function for details
        if ((long)(SEC_TIMER-(*statetime)) > 0) {				// if 1 sec and still waiting
            *state = 4;    //	  abort and re-init
        }
        break;
    case 3:/*SEND*/
        (*statetime) = SEC_TIMER+TIME_OUT;						// reset the timer
        *state = send_packet(sock, buff, bytes, sent);		// see function for details

        break;
    case 4:/*NO WAIT_CLOSE*/
        sock_abort(sock);
        str[0] = '\0';												// close the socket
        *state = 0;														// go back to the INIT state
        printf("%02d:%02d:%02d, \t Port Closed\n",rtc.tm_hour,rtc.tm_min,rtc.tm_sec);
    }
    tcp_tick(sock);
}
MyHandle_A(tcp_Socket* sock, char* buff, int* bytes, int* state, word my_port,
           long* statetime, int *sent, int *rcvd, char* str)
{
    longword  destIP;
    tcp_tick(sock);
    switch(*state) {
    case 0:/*INITIALIZATION*/										// listen for incoming connection
        if( 0L == (destIP = resolve(DEST)) ) {
            printf( "ERROR: Cannot resolve \"%s\" into an IP address\n", DEST );
            exit(2);
        }
        tcp_open(sock,0,destIP,PORT,NULL);

        printf("%02d:%02d:%02d, \t Port %d Initialized\n",rtc.tm_hour,rtc.tm_min,rtc.tm_sec, PORT);
        //while(!sock_established(&socket) && sock_bytesready(&socket)==-1) {
        //   tcp_tick(NULL);
        //   if ((long)(SEC_TIMER-(*statetime)) > 0){			// if 1 sec and no sock
        //	*state = 4;													//	  abort and re-init
        //  break;
        //  }
        //}

        (*statetime) = SEC_TIMER+TIME_OUT;						// reset the statetime
        *sent = *rcvd = 0;											// reset num of packets send and rcvd
        (*state)++;														// init complete move onto next state
        break;
    case 1:/*LISTEN*/
        if(sock_established(sock)) {									// check for a connection
            (*state)++;
            ledOut(4,1);
        }													//   we have connection so move on
        else if ((long)(SEC_TIMER-(*statetime)) > 0) {		// if 1 sec and no sock
            *state = 4;    //	  abort and re-init
        }

        break;
    case 2:/*RECEIVE*/
        *state = receive_packet(sock, buff, bytes, rcvd, str);	// see function for details
        if ((long)(SEC_TIMER-(*statetime)) > 0) {				// if 1 sec and still waiting
            *state = 4;    //	  abort and re-init
        }
        break;
    case 3:/*SEND*/
        (*statetime) = SEC_TIMER+TIME_OUT;						// reset the timer
        *state = send_packet(sock, buff, bytes, sent);		// see function for details

        break;
    case 4:/*NO WAIT_CLOSE*/
        sock_abort(sock);
        str[0] = '\0';												// close the socket
        *state = 0;
        ledOut(4,0);													// go back to the INIT state
        printf("%02d:%02d:%02d, \t Port %d Closed\n",rtc.tm_hour,rtc.tm_min,rtc.tm_sec, PORT);

    }
    tcp_tick(sock);
}
PrintIt(int x, int y, int bytes, int state, word port, long statetime, int rcvd, int sent)
{
    printf ("\x1B=%c%c bytes_A: %05d  state_A: %03d",x,y, bytes, state);
    printf (" my_port_A: %05d  statetime_A: %09ld",port, statetime);
    printf ("\x1B=%c%c Rcvd: %06d  Sent: %06d", x+2, y+1, rcvd, sent);

}

#endif

main()
{
    int inputnum;
    int outputnum;
#ifdef NETWORK
    longword  destIP;
#endif

    save_data[0] = &settings;
    save_lens[0] = sizeof(settings);

    //store = xalloc(65536);		// physical memory address (SRAM)

    keyInit();			// Start-up the keypad driver
    keypadDef();		// Use the default keypad ASCII return values
    brdInit();
    glInit();			// Initialize the graphic driver
    glBackLight(255);	// Turn-on the backlight
    glXFontInit(&fi6x8, 6, 8, 32, 127, Font6x8);			//	Initialize 6x8 font


#ifdef NETWORK
    printf( "initializing sock..\n" );

    sock_init();

    // Wait for the interface to come up
    //while (ifpending(IF_DEFAULT) == IF_COMING_UP) {
    //	tcp_tick(NULL);
    //}

    if(!udp_open(&sock, LOCAL_PORT, resolve(REMOTE_IP), 0, NULL)) {
        printf("udp_open failed!\n");
        exit(0);
    }

#endif
#ifdef IPDOWNLOAD
    UDPDL_Init("Ring");  //Initialize Network Download
#endif

    StatusCode = 0;
    ozonator_loc = 1;
    ozonator_rem = 0;
    CO2_loc = 1;
    CO2_rem = 1;
    flow =0;     //Must be 1 for ozonator to come on.
    purge = 0;
    nighttime_remote = 1;

    for (inputnum=0; inputnum<11; inputnum++) {
        while (anaInEERd(ChanAddr(ADBRD, inputnum)));
    }
    for (outputnum=0; outputnum<8; outputnum++) {
        while (anaOutEERd(ChanAddr(DABRD, outputnum)));
        anaOut(ChanAddr(DABRD, outputnum), 4095);
    }

    anaOutEnable();

    serDopen(19200);
    serCopen(115200);
    serMode(1);
    memPointer = 0;
    //----------------------------------------------------------------------
    // Initialize the variables
    //----------------------------------------------------------------------


    nSel=0;
    chn=0;
    V_OUT_MAX=10; //Maximum output voltage at pressure regulator  *** MOVED TO WIND DIR CHANGE
    V_OUT_MIN=0;  //Minimum output voltage at pressure regulator

    for(i = 0; i < N_CHANNELS+1; i++) {
        byIChA[i]=0;
        byIChB[i]=0;
        feint[i]=0;
        fepro[i]=0;
        fedif[i]=0;
        opSect[i]=1;
        medo[i] = 0;
        vento[i] = 0;
        txlm[i] = 0;
        nContao[i] = 0;

    }
    s45=0;
    f45=0;
    cnt45=0;
    s10=0;
    f10=0;
    cnt10=0;
    outd = 0;
    MOBILE=60;    //60;    //Running average for directional control
    MB=10;        //Running average for wind speed
    ENDCONO=60;	//60
    RECORD=0;

    string_pos_c = 0;
    string_pos_d = 0;
    string_pos_ip = 0;
    SENTENCE_D[0] = '\0';
    SENTENCE_C[0] = '\0';
    SENTENCE_IP[0] = '\0';
    sec_count = 0;
    fDew = 0;

    delay_restart = FALSE;

#ifdef INITIALIZE_PARAMS
    settings[0] = 2;       //channel select or Unit ID
    settings[1] = 1;        //Nighttime local? Assume 1=enable daylight calc
    settings[2] = 0;        //Layer1 type 0=CO2, 1=O3, 2=Off
    settings[3] = 600;      //Set point layer1
    settings[4] = 0;        //DA channel layer1
    settings[5] = 2;        //Layer2 type 0=CO2, 1=O3, 2=Off
    settings[6] = 0;        //Set point layer2
    settings[7] = 0;        //DA channel layer2
    settings[8] = 0;        //Ane. select 0=RMYoung, 1=Sonic
    settings[9] = 1;        //Display? Assume 1=yes, display present
    save_data[0] = &settings;
    save_lens[0] = sizeof(settings);
    writeUserBlockArray(0, save_data, save_lens, 1);
#endif

    //Read UserBlock Variables 3/4/04
    readUserBlockArray(save_data, save_lens, 1, 0);

    printf("settings.[0] = %ld\n", settings[0]);
    printf("settings.[1] = %ld\n", settings[1]);
    printf("settings.[2] = %ld\n", settings[2]);
    printf("settings.[3] = %ld\n", settings[3]);
    printf("settings.[4] = %ld\n", settings[4]);
    printf("settings.[5] = %ld\n", settings[5]);
    printf("settings.[6] = %ld\n", settings[6]);
    printf("settings.[7] = %ld\n", settings[7]);
    printf("settings.[8] = %ld\n", settings[8]);

    UNIT_ID = (int)settings[0];
    nighttime_local = (int)settings[1];
    LAYER[0] = (int)settings[2];
    TARG[0] = (long)settings
              [3];
    //tam0509  if (LAYER[0] == 2) {TARG[0] = 0;}

    DA_Channel[0] = (int)settings[4];
    LAYER[1] = (int)settings[5];
    TARG[1] = (long)settings[6];
    //tam0509  if (LAYER[1] == 2) {TARG[1] = 0;}
    DA_Channel[1] = (int)settings[7];
    if (!settings[8]) {
        Wind_Mult = 12.5;
        Wind_Offset = 0;
    } else {
        Wind_Mult = 10;
        Wind_Offset = 0;  //Changed 6/9/04 - Corn Rings from 2.2 to 0.0

    }
    DISPLAY = (int)settings[9];

    for (i=0; i<N_CHANNELS+1 ; i++) {

        if (LAYER[i] == 2) {
            FAINT[i]=0;   //
            FAPRO[i]=0;     //
            FADIF[i]=0;      //
            FCW[i]=0;          //
            AD_GAIN[i] = 0;
            AD_OFFSET[i] = 0;
            DA_MULT[i] = 0;
        } else if (LAYER[i] == 1) {
            //Layertype 1 - Ozone Settings
            FAINT[i]=-0.00008;   //Initial=0.00002  6/24  -0.00016,-0.0008  6/25 -0.00006  8/9 -0.00008
            FAPRO[i]=-0.0016;     //Initial=0.0008 6/24  -0.0032,-0.0016
            FADIF[i]=-0.016;      //Initial=0.0080
            FCW[i]=0.5;          //Initial=0.2
            AD_GAIN[i] = 50;
            AD_OFFSET[i] = 0;
            DA_MULT[i] = .5;
        } else if (LAYER[i] == 0) {
            //Layertype 0 - CO2 Settings
            FAINT[i]=-0.000015;   //
            FAPRO[i]=-0.0008;     //
            FADIF[i]=-0.016;      //
            FCW[i]=0.15;          //
            AD_GAIN[i] = 400;
            AD_OFFSET[i] = 0;
            DA_MULT[i] = 1;
        }

    }
    LAYERRELAY[0] = 6;  //Define which relay board controls each layer
    LAYERRELAY[1] = 5;

    nighttimeCO2 = 360;




    fFwind=(((((float)anaInVolts(ChanAddr(ADBRD, INP_WSpd)))-
              ((float)anaInVolts(ChanAddr(ADBRD, INP_WDiff)))))* Wind_Mult)-Wind_Offset; //WSpd


    if(fFwind<0) {
        fFwind=0.0;
    }



    for(nI=0; nI<N_CHANNELS+1; nI++) { // Inizializza  contatori

        if(LAYER[nI] == 0) {
            fV0[nI]=V_OUT_MIN+(float)(fFwind*FCW[nI]);
        } else if(LAYER[nI] == 1) {
            fV0[nI]=0;
        } else if(LAYER[nI] == 2) { //OFF
            fV0[nI]=0;
        }

        fFspro[nI]=0;
        fFsdif[nI]=0;
        fFswin[nI]=0;
        fint0[nI]=0;
        nCont01[nI]=1;
        nCont02[nI]=1;
        nCont03[nI]=1;
        nCont04[nI]=1;
        nContao[nI]=0;
        fVp[nI]=fV0[nI];
        if (fVp[nI]>V_OUT_MAX) {
            fVp[nI]=V_OUT_MAX;
        }
        for(wI=1; wI<=DIM_L; wI++)   // Inizializza  contatori

        {
            fFpro0[nI] [wI]=0;
            fFdif0[nI] [wI]=0;
            fFwin[wI]=0;
            fF10win[wI]=0;
        }
    }
    menudisp=0;
    timeOn = (int)SEC_TIMER;
    calc_day(); //Initial Sunrise Calculation


    cont_loop();

}

void cont_loop()
{

    int memo_second;
    int sec_now;


    int Ip;
    float sba1;
    float sba2;

#ifdef NETWORK
    my_port_A = 888;
    my_port_B = 999;
    my_port_C = 777;
    state_A = state_B = state_C = 0;
    sent_A = sent_B = sent_C = 0;
    rcvd_A = rcvd_B = rcvd_C = 0;
    str_A[0]= str_B[0]=str_C[0]= '\0';
#endif


//----------------------------------------------------------------------
//************************   MAIN LOOP    ******************************
//----------------------------------------------------------------------


    while(1) {
        costate {
            keyProcess ();
            waitfor(DelayMs(10));
        }

        costate {
            waitfor ( inkey = keyGet() );		//	Wait for Keypress
            keypress();

        }
        costate {
            readSerialC();
            readSerialD();
#ifdef NETWORK
            MyHandle(&sock_A, buff_A, &bytes_A, &state_A, my_port_A, &statetime_A, &sent_A, &rcvd_A, str_A);
            //PrintIt(0x20, 0x20, bytes_A, state_A, my_port_A, statetime_A, rcvd_A, sent_A);
            MyHandle_A(&sock_B, buff_B, &bytes_B, &state_B, my_port_B, &statetime_B, &sent_B, &rcvd_B, str_B);
            //PrintIt(0x20, 0x24, bytes_B, state_B, my_port_B, statetime_B, rcvd_B, sent_B);
            receive_packet_udp();
#endif
#ifdef IPDOWNLOAD
            if (UDPDL_Tick()) { // this should be called at least twice a second.
                printf("Download request pending!\n");
                // if you need to shut things down before the download, do it here.
            }
#endif

        }
        costate {

            if(timeOn + timeDelay < (int)SEC_TIMER) {
                backlight = 0;
                glBackLight(backlight);
                //printf("%d \n",backlight);

            };

            if (memo_second != (int)SEC_TIMER) {

                anaOutVolts(ChanAddr(DABRD, 7), 3);
                if(ozonator_loc || (ozonator_loc && ozonator_rem)) {
                    ledOut(0,1);
                } else {
                    ledOut(0,0);
                }
                //tam0509  if ((ora>sunr && ora<suns)  && ozonator_rem ) {ledOut(1,1);} else {ledOut(1,0);}
                if ((ora>starttime_ozone && ora<endtime_ozone)  && ozonator_rem ) {
                    ledOut(1,1);
                } else {
                    ledOut(1,0);
                }

                ledOut(2,(flow|purge));
                ledOut(3,1); //Flash watch light
                memo_second = (int)SEC_TIMER;

                wind_avg();
                sector_select();
                fControl();
                set_output();
                second_data();
                if (nContao[0] == ENDCONO) {
                    fDew = (float)anaInVolts(ChanAddr(ADBRD, INP_Dew)); // Read Dew Point
                    minute_average();  //calculate minute averages

                }
                if(ozonator_loc && !ozonator_rem) {
                    ledOut(0,0);
                }
                ledOut(3,0);

                //anaOutVolts(ChanAddr(DABRD, 7), 0);
            }
            hitwd();
        }
    }


}

void sector_select(void)
{

    int i;
    fFlow=360./5.*((float)anaInVolts(ChanAddr(ADBRD, INP_WDir)));    //read wind direction

    Direz=fFlow;
    bySect=(fFlow-22.5)/45+2;
    if(bySect>8) {
        bySect=1;
    }
    MobDir[bySect]++;
    nSel++;

    if (purge) {
        if(purge_off <= (int)SEC_TIMER) {
            digOut(ChanAddr(RELAY, 1),TURNOFF);
            digOut(ChanAddr(RELAY, 2),TURNOFF);
            purge = 0;
        }
    }


    for(chn=0; chn < N_CHANNELS+1; chn++) {

        if(LAYER[chn]==0) { // CO2 CONTROL LAYER
            if (((ora>starttime_co2 && ora<endtime_co2) || (nighttime_remote && nighttime_local)) && CO2_loc && CO2_rem) { // Esegue le operazioni solo di giorno 18.08.2000
                if (nSel>=MOBILE && s45>0.5) {
                    MobSum=-1000;
                    for(wI=1; wI<=8; wI++) {
                        if (MobDir[wI]>MobSum) {
                            MobSum=MobDir[wI];
                            //MobDir[wI]=0;
                            bySect=wI;
                        }                                 //close if
                    }                                    //close for
                    if(bySect>8) {
                        bySect=1;
                    }
                    if(bySect != byIChB[chn]) {
                        digOut(ChanAddr(LAYERRELAY[chn], byIChB[chn]-1),TURNOFF);
                    }
                    digOut(ChanAddr(LAYERRELAY[chn], bySect-1),TURNON);
                    byIChB[chn]=bySect;
                    opSect[chn]=bySect;
                    //nSel=0;

                }                                       //close if

                if  (nSel>=MOBILE && s45<0.5) {
                    for(wI=1; wI<=8; wI++) {
                        digOut(ChanAddr(LAYERRELAY[chn], wI-1),TURNOFF);
                        //MobDir[wI]=0;
                    }
                    byIChA[chn]++;
                    if(byIChA[chn]>8) {
                        byIChA[chn]=1;
                    }
                    digOut(ChanAddr(LAYERRELAY[chn], byIChA[chn]-1),TURNON);

                    opSect[chn]=byIChA[chn];
                    byIChB[chn]=byIChA[chn];
                    //nSel=0;
                    if (fVp[chn] > 1) {
                        fVp[chn]=1;
                    }
                }
            }                                        //close if
            else {

                for(wI=1; wI<=8; wI++) { //Close all sectors
                    digOut(ChanAddr(LAYERRELAY[chn], wI-1),TURNOFF);
                }
            }
        }		//END LAYER SELECT == 0



        if(LAYER[chn]==1) { // OZONE CONTROL LAYER

//tam0509 if ((ora>sunr && ora<suns)  && ozonator_rem )   // Esegue le operazioni solo di giorno 18.08.2000
            if ((ora>starttime_ozone && ora<endtime_ozone)  && ozonator_rem ) { // Esegue le operazioni solo di giorno 18.08.2000
                digOut(ChanAddr(RELAY, 0),TURNON);  //CHANGE2003
            } else {
                digOut(ChanAddr(RELAY, 0),TURNOFF);
            }

            if (delay_restart) { //  Check for timeout if in ozone delay restart
                if (SEC_TIMER > delay_restart_time) {
                    delay_restart = FALSE;
                }
                //printf("%ld %ld \n" ,delay_restart_time, SEC_TIMER);

            }


//tam0509 if ((ora>sunr && ora<suns) && ozonator_loc && ozonator_rem && !purge && !delay_restart)   // Esegue le operazioni solo di giorno 18.08.2000
            if ((ora>starttime_ozone && ora<endtime_ozone) && ozonator_loc && ozonator_rem && !purge && !delay_restart) { // Esegue le operazioni solo di giorno 18.08.2000
                if (nSel>=MOBILE && s45>0.5) {

                    digOut(ChanAddr(RELAY, 1),TURNON); //Turn on AC flow
                    MobSum=-1000;
                    for(wI=1; wI<=8; wI++) {
                        //printf("%f\n",MobSum);
                        if (MobDir[wI]>MobSum) {
                            MobSum=MobDir[wI];
                            //MobDir[wI]=0;
                            bySect=wI;
                        }                                 //close if
                    }                                    //close for
                    if(bySect>8) {
                        bySect=1;
                    }
                    if(bySect != byIChB[chn]) {
                        digOut(ChanAddr(LAYERRELAY[chn], byIChB[chn]-1),TURNOFF);
                    }
                    digOut(ChanAddr(LAYERRELAY[chn], bySect-1),TURNON);
                    byIChB[chn]=bySect;
                    opSect[chn]=bySect;
                    //nSel=0;
                    flow =1;
                    // if (ozonator_rem) digOut(ChanAddr(RELAY, 0),TURNON);  //CHANGE2003
                    //else digOut(ChanAddr(RELAY, 0),TURNOFF);

                }                                       //close if

                if  (nSel>=MOBILE && s45<0.5) {
                    delay_restart = TRUE;
                    delay_restart_time = SEC_TIMER + delay_time;

                    //if (ozonator_rem) digOut(ChanAddr(RELAY, 0),TURNON);  //CHANGE2003
                    //else digOut(ChanAddr(RELAY, 0),TURNOFF);

                    if(flow) {
                        purge = 1;
                        flow = 0;
                        purge_off = (int)SEC_TIMER + PURGE_TIME;
                    }

                }




            }                                        //close if
            else {
                //digOut(ChanAddr(RELAY, 0),TURNOFF);  //Turn off Ozone Generator
                if  (s45<0.5) {
                    delay_restart = TRUE;
                    delay_restart_time = SEC_TIMER + delay_time;
                }

                if(flow) {
                    purge = 1;
                    flow = 0;
                    purge_off = (int)SEC_TIMER + PURGE_TIME;
                }



            }

        }		//END LAYER SELECT == 1

        if(LAYER[chn]== 2 ) { // LAYER OFF
            for(wI=1; wI<=8; wI++) { //Close all sectors
                digOut(ChanAddr(LAYERRELAY[chn], wI-1),TURNOFF);
            }
        }



    }		//END CHN LOOP
    if (nSel>=MOBILE) {
        nSel = 0;
        for(wI=1; wI<=8; wI++) {
            MobDir[wI]=0;
        }
    }
}		//END SUB SECTOR SELECT

/************************************************************/
/*                    average wind speed                    */
/************************************************************/


void wind_avg(void)
{
    fFwind=(((((float)anaInVolts(ChanAddr(ADBRD, INP_WSpd)))-
              ((float)anaInVolts(ChanAddr(ADBRD, INP_WDiff)))))* Wind_Mult)-Wind_Offset; //WSpd Diff Input
    //fFwind = 5; //TEST
    f45=fFwind/45.;
    f10=fFwind/10.;

    if (cnt45<45) {
        s45=s45+f45;
        cnt45++;
    } else {
        s45=s45-s45/45.+f45;
    }
    if (cnt10<10) {
        s10=s10+f10;
        cnt10++;
    } else {
        s10=s10-s10/10.+f10;
    }
    fwind=fFwind;

}








/************************************************************/
/*                    fControl                              */
/************************************************************/
void fControl(void)
{
    for(chn=0; chn < N_CHANNELS+1; chn++) {

        //   read analog input *Differential Input Change
        fResA=0;
        for (wI=1; wI<=100; wI++) {
            fIrgaA=((float)anaInVolts(ChanAddr(ADBRD, (chn*2))));
            fIrgaB =((float)anaInVolts(ChanAddr(ADBRD, (chn*2+1))));
            fResA=fResA+(((fIrgaA-fIrgaB)* AD_GAIN[chn])+ AD_OFFSET[chn]);
        }
        fpps[chn]=fResA/100;
        //printf("%.2f \n",fpps[chn]);


        nContao[chn]++;
        if(LAYER[chn]==0) {
            if ((ora<starttime_co2 || ora>endtime_co2) && (nighttime_remote && nighttime_local)) {
                TARG[chn]=  nighttimeCO2;
            } else {
                if (chn == 0) {
                    TARG[chn] = settings[3];
                }
                if (chn == 1) {
                    TARG[chn] = settings[6];
                }

            }
        }
        fval[chn] = fpps[chn] - TARG[chn];

        if(LAYER[chn]==0) { // CO2 CONTROL LAYER
            //if (ora<sunr | ora>suns | !nighttime_remote | !nighttime_local | !CO2_rem | !CO2_loc)   // If nighttime_remote
            if(((ora<starttime_co2 || ora>endtime_co2) && !(nighttime_remote && nighttime_local))|| !CO2_rem || !CO2_loc ) {
                fVs[chn]=0;
            } else { // Daytime
                if (nCont01[chn] <= DIM_L) { //Delay for first 45 seconds

                    fFpro0[chn][nCont01[chn]] = fval[chn];
                    if (nCont01[chn] == 1) {
                        fFdif0[chn][nCont01[chn]] = fFpro0[chn][nCont01[chn] - 0];
                    }
                    if (nCont01[chn] > 1) {
                        fFdif0[chn][nCont01[chn]] = fFpro0[chn][nCont01[chn]] - fFpro0[chn][nCont01[chn] - 1];
                    }

                    fFspro[chn] = fFspro[chn] + fFpro0[chn][nCont01[chn]];
                    fFsdif[chn] = fFsdif[chn] + fFdif0[chn][nCont01[chn]];

                    if ((fVp[chn] > V_OUT_MIN) && (fVp[chn] < V_OUT_MAX)) {
                        fint0[chn] = fint0[chn] + FAINT[chn] * fval[chn];
                    }
                    if ((fVp[chn] == V_OUT_MIN) && (fval < 0)) {
                        fint0[chn] = fint0[chn] + FAINT[chn] * fval[chn];
                    }
                    if ((fVp[chn] == V_OUT_MAX) && (fval > 0)) {
                        fint0[chn] = fint0[chn] + FAINT[chn] * fval[chn];
                    }

                    nCont01[chn]++;
                    fVs[chn]=fVp[chn];
                } else {
                    fFspro[chn] = fFspro[chn] - fFpro0[chn][ nCont02[chn]];
                    fFsdif[chn] = fFsdif[chn] - fFdif0[chn][ nCont02[chn]];
                    fFpro0[chn][ nCont02[chn]] = fval[chn];

                    if (nCont02[chn] == 1) {
                        fFdif0[chn][nCont02[chn]] = fFpro0[chn][nCont02[chn]] - fFpro0[chn][ (DIM_L-1)];
                    }

                    if (nCont02[chn] > 1) {
                        fFdif0[chn][nCont02[chn]] = fFpro0[chn][nCont02[chn]] - fFpro0[chn][ nCont02[chn] - 1];
                    }

                    fFspro[chn] = fFspro[chn] + fFpro0[chn][nCont02[chn]];
                    fFsdif[chn] = fFsdif[chn] + fFdif0[chn][nCont02[chn]];
                    nCont02[chn]++;
                    nCont04[chn]++;

                    if (nCont02[chn] > DIM_L) {
                        nCont02[chn] = 1;
                    }

                    if (nCont04[chn] > MB) {
                        nCont04[chn] = 1;
                    }

                    fmpro[chn] = fFspro[chn] / DIM_L;
                    fmdif[chn] = fFsdif[chn] / DIM_L;
                    fmwin[chn] = s45;
                    fm10w[chn] = s10;

                    if ((fVp[chn] > V_OUT_MIN) && (fVp[chn] <V_OUT_MAX)) {
                        fint0[chn] = (float)(fint0[chn] + FAINT[chn] * fval[chn]);
                    }

                    if ((fVp[chn] == V_OUT_MIN) && (fval < 0)) {
                        fint0[chn] = (float)(fint0[chn] + FAINT[chn] * fval[chn]);
                    }

                    feint[chn] = fint0[chn];
                    fepro[chn] = (float)(FAPRO[chn] * fmpro[chn]);
                    fedif[chn] = (float)(FADIF[chn] * fmdif[chn]);
                    fcor[chn] = feint[chn] + fepro[chn] + fedif[chn];

                    if (fmwin[chn] == 0) {
                        fVp[chn] = fV0[chn] + fcor[chn];
                    } else {
                        fVp[chn] = (float)(fV0[chn] + fcor[chn] + ((FCW[chn] * fmwin[chn] * fm10w[chn]) / fmwin[chn]));
                    }

                    if (fVp[chn] < V_OUT_MIN) {
                        fVp[chn] = V_OUT_MIN;
                    }
                    if (s45<0.5) {
                        if (fVp[chn] > 1) {
                            fVp[chn] =1;
                        }
                    }

                    if (fVp[chn] > V_OUT_MAX) {
                        fVp[chn] = V_OUT_MAX;
                        //CO2_rem = 0;

                    }
                    fVs[chn]=16/5 * fVp[chn] + 4; // JAM 20170726 Scale the mA to V.
                    printf("fVs is %f \n fVp is %f \n", fVs[chn], fVp[chn]);
                }
            }
        }  //End Layertype=0

        if(LAYER[chn]==1) { // O3 CONTROL LAYER
            //tam0509 if (ora<sunr || ora>suns || !ozonator_loc || !ozonator_rem || !flow  )   // If nighttime or ozone off
            if (ora<starttime_ozone || ora>endtime_ozone || !ozonator_loc || !ozonator_rem || !flow  ) { // If nighttime or ozone off
                fVs[chn]=0;
            } else { // Daytime and ozone on
                if ((nCont01[chn] <= DIM_L)&&flow) { //Delay for first 45 seconds
                    fFpro0[chn][nCont01[chn]] = fval[chn];
                    if (nCont01[chn] == 1) {
                        fFdif0[chn][nCont01[chn]] = fFpro0[chn][nCont01[chn] - 0];
                    }
                    if (nCont01[chn] > 1) {
                        fFdif0[chn][nCont01[chn]] = fFpro0[chn][nCont01[chn]] - fFpro0[chn][nCont01[chn] - 1];
                    }

                    fFspro[chn] = fFspro[chn] + fFpro0[chn][nCont01[chn]];
                    fFsdif[chn] = fFsdif[chn] + fFdif0[chn][nCont01[chn]];

                    if ((fVp[chn] > V_OUT_MIN) && (fVp[chn] < V_OUT_MAX)) {
                        fint0[chn] = fint0[chn] + FAINT[chn] * fval[chn];
                    }
                    if ((fVp[chn] == V_OUT_MIN) && (fval < 0)) {
                        fint0[chn] = fint0[chn] + FAINT[chn] * fval[chn];
                    }
                    if ((fVp[chn] == V_OUT_MAX) && (fval > 0)) {
                        fint0[chn] = fint0[chn] + FAINT[chn] * fval[chn];
                    }

                    nCont01[chn]++;
                    fVs[chn]=fVp[chn];
                } else {
                    fFspro[chn] = fFspro[chn] - fFpro0[chn][ nCont02[chn]];
                    fFsdif[chn] = fFsdif[chn] - fFdif0[chn][ nCont02[chn]];
                    fFpro0[chn][ nCont02[chn]] = fval[chn];

                    if (nCont02[chn] == 1) {
                        fFdif0[chn][nCont02[chn]] = fFpro0[chn][nCont02[chn]] - fFpro0[chn][ (DIM_L-1)];
                    }

                    if (nCont02[chn] > 1) {
                        fFdif0[chn][nCont02[chn]] = fFpro0[chn][nCont02[chn]] - fFpro0[chn][ nCont02[chn] - 1];
                    }

                    fFspro[chn] = fFspro[chn] + fFpro0[chn][nCont02[chn]];
                    fFsdif[chn] = fFsdif[chn] + fFdif0[chn][nCont02[chn]];
                    nCont02[chn]++;
                    nCont04[chn]++;

                    if (nCont02[chn] > DIM_L) {
                        nCont02[chn] = 1;
                    }

                    if (nCont04[chn] > MB) {
                        nCont04[chn] = 1;
                    }

                    fmpro[chn] = fFspro[chn] / DIM_L;
                    fmdif[chn] = fFsdif[chn] / DIM_L;
                    fmwin[chn] = s45;
                    fm10w[chn] = s10;

                    if ((fVp[chn] > V_OUT_MIN) && (fVp[chn] <V_OUT_MAX)) {
                        fint0[chn] = (float)(fint0[chn] + FAINT[chn] * fval[chn]);
                    }

                    if ((fVp[chn] == V_OUT_MIN) && (fval < 0)) {
                        fint0[chn] = (float)(fint0[chn] + FAINT[chn] * fval[chn]);
                    }

                    feint[chn] = fint0[chn];
                    fepro[chn] = (float)(FAPRO[chn] * fmpro[chn]);
                    fedif[chn] = (float)(FADIF[chn] * fmdif[chn]);
                    fcor[chn] = feint[chn] + fepro[chn] + fedif[chn];

                    if (fmwin[chn] == 0) {
                        fVp[chn] = fV0[chn] + fcor[chn];
                    } else {
                        fVp[chn] = (float)(fV0[chn] + fcor[chn] + ((FCW[chn] * fmwin[chn] * fm10w[chn]) / fmwin[chn]));
                    }

                    if (fVp[chn] < V_OUT_MIN) {
                        fVp[chn] = V_OUT_MIN;
                    }

                    if (fVp[chn] > V_OUT_MAX) {
                        fVp[chn] = V_OUT_MAX;
                        // tam0520  ozonator_loc = 0;
                        // tam0520 fVp[chn]=0;        //Changed 05/05

                    }
                    fVs[chn]=fVp[chn];
                }
            }  //End Layertype=1

        }  //Chiude il loop della notte 18.08.2000
        if(LAYER[chn]==2) { // LAYER OFF
            fVp[chn] = 0;
            fVs[chn]=fVp[chn];
        }



        txlm[chn] = txlm[chn] + fVp[chn];
        medo[chn] = medo[chn] + fpps[chn];
        vento[chn] = fwind + vento[chn];
    }


    return;
}


void set_output(void)
{
    for(chn=0; chn < N_CHANNELS+1; chn++) {
        if(LAYER[chn]==0) { // CO2 CONTROL LAYER
            if ((ora<starttime_co2 || ora>endtime_co2) && !(nighttime_remote && nighttime_local)) { // Esegue le operazioni solo di giorno 18.08.2000
                anaOutmAmps(ChanAddr(DABRD, chn), 0);
                digOut(ChanAddr(RELAY, 2),TURNOFF);  //CO2 shutoff valve
            } else {
                anaOutmAmps(ChanAddr(DABRD, DA_Channel[chn]), (fVs[chn]*DA_MULT[chn]));
                digOut(ChanAddr(RELAY, 2),TURNON);  //CO2 shutoff valve
            }
        }
        if(LAYER[chn]==1) { // O3 CONTROL LAYER
            //tam0509 if (ora<sunr || ora>suns)   // Esegue le operazioni solo di giorno 18.08.2000
            if (ora<starttime_ozone || ora>endtime_ozone) { // Esegue le operazioni solo di giorno 18.08.2000
                anaOutmAmps(ChanAddr(DABRD, DA_Channel[chn]), 0);
            } else {
                anaOutmAmps(ChanAddr(DABRD, DA_Channel[chn]), (fVs[chn]*DA_MULT[chn]));
            }
        }
    }






    /*        if ((ora<sunr | ora>suns) && !nighttime) //It's time to sleep at night
                 {
                 for(nI=0; nI<=N_CHANNELS;nI++)    //Zero analog output
                 	{
    	     	  		anaOutVolts(ChanAddr(DABRD, nI), 0);

                 	}
                 }
          	else
          	      //It's time to work during the day
                 {
                 for(nI=0; nI<=N_CHANNELS;nI++)
                 	{
                 	anaOutVolts(ChanAddr(DABRD, DA_Channel[nI]), (fVs[nI]*DA_MULT[nI]));

                 	}
                 }
    */
}

void second_data(void)
{
    char mess[20];
    auto int retval1;
    tm_rd(&rtc);
    sec_count++;
    if(sec_count >= GRAB_TIME) {
#ifdef NETWORK
        sprintf(mess, "H,%d,%.2f\r",(flow|purge),fVs[1]);
        sock_write(&sock_B,mess,strlen(mess));
#endif
        sprintf(message,"S,%d,%02d:%02d:%02d,%.2f,%.0f,%.0f,%.2f,%.0f,%.0f,%.1f,%.0f,%d \r",UNIT_ID,rtc.tm_hour,rtc.tm_min,rtc.tm_sec,fVp[0],fpps[0],TARG[0],fVp[1],fpps[1],TARG[1],fFwind,Direz,opSect[0]);
        serDputs(message);
        while (serDwrFree() != DOUTBUFSIZE) ;

        if(!Polled) { //Send data to serial port
            serCputs(message);
            while (serCwrFree() != COUTBUFSIZE) ;

        }
#ifdef NETWORK
        if(state_A >> 1) {
            //for(i=0;i<20;i++){
            retval1 = sock_fastwrite(&sock_A,message,strlen(message));
        }
#endif

        sec_count = 0;
    }

    if(!menudisp) {
        glPrintf(0,0, &fi6x8, "%02d/%02d/%04d  %02d:%02d:%02d", rtc.tm_mon,rtc.tm_mday,(1900 + rtc.tm_year),rtc.tm_hour,rtc.tm_min,rtc.tm_sec);
        glPrintf(0,8, &fi6x8, "Wind:%4.1f/%3.0f Sect:%d",fFwind,Direz,opSect[0]);
        //glPrintf(0,16, &fi6x8, "1)%s%4.0f Out:%5.2f",treatments[LAYER[0]],fpps[0], fVs[0]);
        //glPrintf(0,24, &fi6x8, "1)TARG:%4.0f ",TARG[0]);
        glPrintf(0,16, &fi6x8, "%s%4.0f/%4.0f V:%4.2f",treatments[LAYER[0]],fpps[0],TARG[0], fVs[0]);
        glPrintf(0,24, &fi6x8, "%s%4.0f/%4.0f V:%4.2f",treatments[LAYER[1]],fpps[1],TARG[1], fVs[1]);

    }
}






//*************************************************************************************************************************************************************************************************
//******    Minute averages
//*************************************************************************************************************************************************************************************************
void minute_average(void)
{
    auto int retval1;
    calc_day();
    for(chn=0; chn < N_CHANNELS+1; chn++) {
        txlm[chn] = txlm[chn] / nContao[chn];
        vento[chn] = vento[chn] / nContao[chn];
        medo[chn] = medo[chn] / nContao[chn];
    }
    //fDew = fDew/nContao[0];

    (int)StatusCode = ozonator_loc && ozonator_rem && flow;
    //printf("%d,%d,%d \n",ozonator_loc, ozonator_rem, (int)StatusCode);
    if(!Polled) { // Write avg to serial port
        sprintf(message,"M,%d,%02d/%02d/%04d,%02d:%02d:%02d,%.2f,%.0f,%.0f,%.2f,%.0f,%.0f,%.1f,%.0f,%d,%d,%.2f \r",UNIT_ID,rtc.tm_mon,rtc.tm_mday,(1900 + rtc.tm_year),rtc.tm_hour,rtc.tm_min,rtc.tm_sec,txlm[0],medo[0],TARG[0],txlm[1],medo[1],TARG[1],vento[0],fFlow,opSect[0],StatusCode,fDew);
        serCputs(message);
        while (serCwrFree() != COUTBUFSIZE) ;
#ifdef NETWORK
        if(state_A >> 1) {
            //for(i=0;i<20;i++){
            retval1 = sock_fastwrite(&sock_A,message,strlen(message));
        }
#endif

    }

    for(chn=0; chn < N_CHANNELS+1; chn++) {
        if(LAYER[chn]==1) {
            if (fVp[chn] > V_OUT_MAX) {
                fVp[chn] = V_OUT_MAX;
                //ozonator_loc = 0;  //Changed 05/05  Moving to Minute Average
                //fVp[chn]=0;        //Changed 05/05

            }
        }
    }




    //fMedo=medo[chn]; // CO2 avg

    //fResA=fVp[chn]; // Output Volts



    if(RECORD > 2975) {
        RECORD=0;
    }
    //write_mem(RECORD);
    RECORD = RECORD + 1;

    for(chn=0; chn < N_CHANNELS+1; chn++) {

        medo[chn] = 0;
        vento[chn] = 0;
        txlm[chn] = 0;
        nContao[chn] = 0;
    }
    fDew = 0;


}


void calc_day(void)
{
    double d, m, y;
    tm_rd(&rtc);
    y = 2005;

    CalcSun(rtc.tm_year, rtc.tm_mon, rtc.tm_mday, 40.04,-88.13,-6);
//printf("%d,%d,%d,%d \n",d,m,riset,settm);
    sunr = (float)riset;
    suns = (float)settm;
    ora=rtc.tm_hour+(float)(rtc.tm_min/60.);
//printf("%f %f %f \n",sunr, suns, ora);

}

void Reset_Ozone_Vars(void)
{

    for(nI=0; nI<N_CHANNELS+1; nI++) { // Inizializza  contatori
        if(LAYER[nI] == 1) {
            fV0[nI]=0;
            //byIChA[nI]=0;
            //byIChB[nI]=0;
            feint[nI]=0;
            fepro[nI]=0;
            fedif[nI]=0;
            fFspro[nI]=0;
            fFsdif[nI]=0;
            fFswin[nI]=0;
            fint0[nI]=0;
            nCont01[nI]=1;
            nCont02[nI]=1;
            nCont03[nI]=1;
            nCont04[nI]=1;
            nContao[nI]=0;
            txlm[nI] = 0;
            vento[nI] = 0;
            medo[nI] = 0;

            fVp[nI]=fV0[nI];
            if (fVp[nI]>V_OUT_MAX) {
                fVp[nI]=V_OUT_MAX;
            }
            for(wI=1; wI<=DIM_L; wI++)   // Inizializza  contatori

            {
                fFpro0[nI] [wI]=0;
                fFdif0[nI] [wI]=0;
                fFwin[wI]=0;
                fF10win[wI]=0;
            }
        }
    }

}

/************************************************************/
/*                    Calibrate                             */
/************************************************************/
void calibrate(int sector)
{
    float temp;
    int memo_sec1;
    int memo_sec2;
    int m;


    anaOutmAmps(ChanAddr(DABRD, DA_Channel[0]), 10);
    anaOutmAmps(ChanAddr(DABRD, DA_Channel[1]), 10);
    digOut(ChanAddr(RELAY, 1),TURNON); //Turn on AC flow
    //anaOutVolts(ChanAddr(DABRD, 0), 5);  //Set Control Valve at 50%
    //anaOutVolts(ChanAddr(DABRD, 1), 5);  //Set Control Valve at 50%
top:
    for(m=0; m<8; m++) {		//Turn all valves off
        digOut(ChanAddr(DIGIO, m),TURNOFF);
        digOut(ChanAddr(5, m),TURNOFF);
    }
    memo_sec2 = (int)SEC_TIMER+5;

    while(1) {

        if(sector == 0) {
            for(m=0; m<8; m++) {
                digOut(ChanAddr(DIGIO, m),TURNON);
                digOut(ChanAddr(5, m),TURNON);
                while(1) {
                    costate {
                        keyProcess ();
                        waitfor(DelayMs(10));
                    }

                    costate {
                        waitfor ( inkey = keyGet() );		//	Wait for Keypress
                        switch (inkey) {
                        case 'E':
                            exit(-1);
                            menudisp=1;
                            cont_loop();

                            break;

                        case 'R':
                            sector = 1;
                            goto top;
                            break;

                        case 'L':
                            sector = 8;
                            goto top;
                            break;

                        default:
                            break;
                        }



                    }

                    costate {

                        if (memo_sec1 != (int)SEC_TIMER) {
                            hitwd();
                            temp=(((float)anaInVolts(ChanAddr(ADBRD, 0))) * 400)  ;

                            glPrintf(0,0, &fi6x8, "   <<CALIBRATE>>");
                            glPrintf(0,8, &fi6x8, "Sector : %d",m+1);
                            glPrintf(0,16, &fi6x8, "1)CO2:%4.0f Out:5.00",temp);
                            //glPrintf(0,24, &fi6x8, "2)CO2:%4.0f Out:%5.2f",fpps[1], fVs[1]);


                            memo_sec1 = (int)SEC_TIMER;
                        }
                        if (memo_sec2 <= (int)SEC_TIMER) {
                            digOut(ChanAddr(DIGIO, m),TURNOFF);
                            digOut(ChanAddr(5, m),TURNOFF);
                            memo_sec2 = (int)SEC_TIMER+5;
                            break;
                        }
                    }
                }
            }
        }


        else {
            m = sector -1;
            digOut(ChanAddr(DIGIO, m),TURNON);
            digOut(ChanAddr(5, m),TURNON);
            while(1) {
                costate {
                    keyProcess ();
                    waitfor(DelayMs(10));
                }

                costate {
                    waitfor ( inkey = keyGet() );		//	Wait for Keypress

                    switch(inkey) {
                    case 'E':
                        exit(-1);
                        menudisp=1;
                        cont_loop();
                        break;

                    case 'R':
                        ++sector;
                        if(sector == 9) {
                            sector = 1;
                        }

                        goto top;
                        break;

                    case 'L':
                        --sector;
                        if(sector == 0) {
                            sector = 8;
                        }

                        goto top;
                        break;

                    default:
                        break;
                    }

                    cont_loop();

                }

                costate {
                    readSerialD();
                    if (memo_sec1 != (int)SEC_TIMER) {
                        //hitwd();
                        temp=(((float)anaInVolts(ChanAddr(ADBRD, 0))) * 400)  ;
                        glPrintf(0,0, &fi6x8, "   <<CALIBRATE>>");
                        glPrintf(0,8, &fi6x8, "Sector : %d",sector);
                        glPrintf(0,16, &fi6x8, "1)CO2:%4.0f Out:5.00",temp);
                        //glPrintf(0,24, &fi6x8, "2)CO2:%4.0f Out:%5.2f",fpps[1], fVs[1]);

                        memo_sec1 = (int)SEC_TIMER;
                    }
                }
            }
        }
    }



}

void readSerialC(void)
{
    input_char = serCgetc();
    //printf("%c \n",input_char);

    if(input_char == '\r') {
        SENTENCE_C[string_pos_c++] = '\0';

        readString_C();
    } else if(input_char > 100) {
    }

    else if(input_char > 0) {

        SENTENCE_C[string_pos_c] = input_char;
        string_pos_c++;
        //printf("%d \t %c \n",input_char,input_char);
    }
}


void readSerialD(void)
{
    input_char = serDgetc();
    //printf("%c \n",input_char);

    if(input_char == '\r') {
        SENTENCE_D[string_pos_d++] = '\0';

        readString_D();
    } else if(input_char > 100) {
    }

    else if(input_char > 0) {

        SENTENCE_D[string_pos_d] = input_char;
        string_pos_d++;
        //printf("%d \t %c \n",input_char,input_char);
    }
}


void readString_C(void)
{
    int i;
    int location;
    printf("%s \n",SENTENCE_C);
    token = strtok(SENTENCE_C,delim);

    switch(*token) { // Read Command
    case 65: //A Set CO2 local
        string_pos_c = 0;
        token = strtok(NULL,delim);
        if (atoi(token) == UNIT_ID) {
            token = strtok(NULL,delim);
            CO2_loc = atoi(token);
        }


        break;
    case 66: //B Set CO2 remote
        string_pos_c = 0;
        token = strtok(NULL,delim);
        CO2_rem = atoi(token);

        break;


    case 67: //C Set CO2 Nightime
        string_pos_c = 0;
        token = strtok(NULL,delim);
        nighttime_remote = atoi(token);
        token = strtok(NULL,delim);
        nighttimeCO2 = atoi(token);
        break;

    case 68: //D Download
        printf("Download \n");
        string_pos_c = 0;
        //download(RECORD);
        break;



    case 78: //N Set Ozone Remote
        string_pos_c = 0;
        token = strtok(NULL,delim);
        if (atoi(token) == UNIT_ID) {
            string_pos_c = 0;
            token = strtok(NULL,delim);
            ozonator_loc =  atoi(token);
        }

        break;



    case 79: //Set Ozone
        token = strtok(NULL,delim);
        ozonator_rem = atoi(token);	//set ozone
        //tam0509 if ((ora>sunr && ora<suns)  && ozonator_rem ) digOut(ChanAddr(RELAY, 0),TURNON);
        if ((ora>starttime_ozone && ora<endtime_ozone)  && ozonator_rem ) {
            digOut(ChanAddr(RELAY, 0),TURNON);
        } else {
            digOut(ChanAddr(RELAY, 0),TURNOFF);
        }

        token = strtok(NULL,delim);
        for(i=0; i < N_CHANNELS+1; i++) {

            if(LAYER[i] == 1) {
                //nCont01[i]=1;
                //nCont02[i]=1;
                //nCont03[i]=1;
                //nCont04[i]=1;
                //nContao[i]=0;
                //medo[i] = 0;
                //vento[i] = 0;
                //txlm[i] = 0;
                //nContao[i] = 0;
                //tam0509 TARG[i] = (atof(token)* OZONE_MULT);
                //tam0509 if(TARG[i] > OZONE_MAX) TARG[i] = OZONE_MAX;
            }
            fDew = 0;
        }
        string_pos_c = 0;
        break;
        // *** Added 03/27/04
    case 82:   //Reset Controller (R)
        string_pos_c = 0;
        token = strtok(NULL,delim);
        if (atoi(token) == UNIT_ID) {
            exit(0);
        }
        break;

    case 83: //Transmit second data (S)
        string_pos_c = 0;
        token = strtok(NULL,delim);
        if (atoi(token) == UNIT_ID) {
            sprintf(message,"S,%d,%02d:%02d:%02d,%.2f,%.0f,%.0f,%.2f,%.0f,%.0f,%.1f,%.0f,%d \r",UNIT_ID,rtc.tm_hour,rtc.tm_min,rtc.tm_sec,fVs[0],fpps[0],TARG[0],fVs[1],fpps[1],TARG[1],fFwind,Direz,opSect[0]);
            serCputs(message);
            while (serCwrFree() != COUTBUFSIZE) ;
        }

        break;


    case 84: //Set Time
        string_pos_c = 0;
        token = strtok(NULL,delim);
        rtc.tm_mon = atoi(token);	//set month
        token = strtok(NULL,delim);
        rtc.tm_mday = atoi(token); //set day
        token = strtok(NULL,delim);
        rtc.tm_year = (atoi(token)-1900);	//set year
        token = strtok(NULL,delim);
        rtc.tm_hour = atoi(token); //set hour
        token = strtok(NULL,delim);
        rtc.tm_min = atoi(token); //set minute
        token = strtok(NULL,delim);
        rtc.tm_sec = atoi(token); //set second

        tm_wr(&rtc);
        SEC_TIMER = mktime(&rtc);
        sprintf(message,"T,%02d/%02d/%04d,%02d:%02d:%02d \r",rtc.tm_mon,rtc.tm_mday,(rtc.tm_year+1900),rtc.tm_hour,rtc.tm_min,rtc.tm_sec);
        serDputs(message);
        while (serDwrFree() != DOUTBUFSIZE) ;
        calc_day();
        break;

    case 86: //V Reset Ozone Variables
        string_pos_c = 0;
        Reset_Ozone_Vars();
        break;

    case 87: //W Hard Reset Ozone Rings
        string_pos_c = 0;
        token = strtok(NULL,delim);
        if (atoi(token) == UNIT_ID) {
            Reset_Ozone_Vars();
            printf("Reset Ozone Variables this ring only\n");
        }


        break;




    default:


    }
    string_pos_c = 0;
}

void readString_D(void)
{
    int location;
    //printf("%s \n",SENTENCE_D);
    token = strtok(SENTENCE_D,delim);

    switch(*token) { // Read Command

    case 67: //Calibrate
        string_pos_d = 0;
        token = strtok(NULL,delim);
        //printf("%d \n",atoi(token));
        calibrate(atoi(token));
        break;

    case 68: //Download
        printf("Download \n");
        string_pos_d = 0;
        //download(RECORD);
        break;

    case 82:
        string_pos_d = 0;

        cont_loop();
        break;

    case 84: //Set Time
        string_pos_d = 0;
        token = strtok(NULL,delim);
        rtc.tm_mon = atoi(token);	//set month
        token = strtok(NULL,delim);
        rtc.tm_mday = atoi(token); //set day
        token = strtok(NULL,delim);
        rtc.tm_year = (atoi(token)-1900);	//set year
        token = strtok(NULL,delim);
        rtc.tm_hour = atoi(token); //set hour
        token = strtok(NULL,delim);
        rtc.tm_min = atoi(token); //set minute
        token = strtok(NULL,delim);
        rtc.tm_sec = atoi(token); //set second

        tm_wr(&rtc);
        SEC_TIMER = mktime(&rtc);
        calc_day();
        break;

    case 83: //Transmit second data (S)
        string_pos_d = 0;
        token = strtok(NULL,delim);
        if (atoi(token) == UNIT_ID) {
            sprintf(message,"S,%d,%02d:%02d:%02d,%.2f,%.0f,%.0f,%.2f,%.0f,%.0f,%.1f,%.0f,%d \r",UNIT_ID,rtc.tm_hour,rtc.tm_min,rtc.tm_sec,fVs[0],fpps[0],TARG[0],fVs[1],fpps[1],TARG[1],fFwind,Direz,opSect[0]);
            serDputs(message);
            while (serDwrFree() != DOUTBUFSIZE) ;
        }

        break;

    default:


    }
    string_pos_d = 0;
}

void keypress(void)
{
    printf("Key Pressed \n");

    if((int)backlight == 0) {
        printf("Key Pressed - Light On \n");

        backlight = 1;
        glBackLight(1);
        printf("%d \n", backlight);
        timeOn = (int)SEC_TIMER;
        return ;
    }
    switch (inkey) {
    case 'E':
        menudisp=0;
        DispMenu();
        break;

    default:

        break;
    }

}


//------------------------------------------------------------------------
// Milli-sec delay function
//------------------------------------------------------------------------
nodebug
void msDelay(long sd)
{
    auto unsigned long t1;

    t1 = MS_TIMER + sd;
    while ((long)(MS_TIMER-t1) < 0);
}


//------------------------------------------------------------------------
// Process key to do number and ASCII field changes
//------------------------------------------------------------------------
int ProcessKeyField(int mode, fieldupdate *field)
{
    static int wKey;

    keyProcess();
    msDelay(100);
    if((wKey = keyGet()) != 0) {
        switch(wKey) {
            // Decrement number by 10 or pointer by 3
        case '-':
            if(mode == NUMBER) {
                field->data -= 10;
            } else {
                field->ptr  -= 3;
            }
            break;

            // Increment number by 10 or pointer by 3
        case '+':
            if(mode == NUMBER) {
                field->data += 10;
            } else {
                field->ptr  += 3;
            }
            break;

            // Increment number or pointer by 1
        case 'U':
            if(mode == NUMBER) {
                field->data++;
            } else {
                field->ptr++;
            }
            break;

            // Decrement number or pointer by 1
        case 'D':	// Decrement X1
            if(mode == NUMBER) {
                field->data--;
            } else {
                field->ptr--;
            }
            break;

            // Done Editing field
        case 'E':
            wKey = 'E';
            break;

        default:
            wKey = -1;
            break;
        }
    }
    return(wKey);
}


//------------------------------------------------------------------------
// Get and process the users MENU option
//------------------------------------------------------------------------
int GetKeypadOption( int *offset, int *highlight, int msize )
{
    static int wKey;

    if((wKey = keyGet()) != 0) {
        switch(wKey) {
        case '-':	// Page down
            if(*offset < (((msize)/sizeof(int)) - 1)) {
                if((*offset + MAXDISPLAYROWS) < (((msize)/sizeof(int)) - 1)) {
                    *offset += 4;
                }
            }
            if(*offset == 0) {
                *highlight = 1;
            } else {
                *highlight = 0;
            }
            wKey = -1;
            break;

        case '+':	// Page up
            if(*offset > 3) {
                *offset -=4;
            } else {
                *offset = 0;
            }
            if(*offset == 0) {
                *highlight = 1;
            } else {
                *highlight = 0;
            }
            wKey = -1;
            break;

        case 'U':	// Scroll-up by one line
            *highlight -= 1;
            if(*highlight < 0) {
                *offset -= 1;
                *highlight = 0;
            }
            if(*offset == 0 && *highlight == 0) {
                *highlight = 1;
            }
            wKey = -1;
            break;

        case 'D':	// Scroll-down by one line
            if((*offset + (*highlight) + 1) < (((msize)/sizeof(int)) - 1)) {
                *highlight += 1;
                if(*highlight > 3) {
                    *offset += 1;
                    *highlight = 3;
                }
            }
            wKey = -1;
            break;

        case 'E':	// Select option
            wKey = *offset + *highlight;
            break;

        default:
            wKey = -1;
            break;
        }
    }
    return(wKey);
}

//------------------------------------------------------------------------
// Display a MENU on the LCD display and get the menu option from the user
//------------------------------------------------------------------------
int display_menu ( char **line, int initialize, int menusize)
{
    static int offset, tmpoffset, i;
    static int menu_option;
    static int lasthighlight;
    static int scrolling;
    static int highlight;

    costate {
        if(initialize) {
            offset = 0;				// Initialize menu line index
            highlight = 1;			// Assumes all menus have a heading
            tmpoffset = ~offset;
        }

        menu_option = 0;			// Initially set to no option selected
        scrolling = FALSE;

        // Wait until you get a valid user option
        while (menu_option == 0) {
            // Display menu option
            if(tmpoffset != offset) {
                glBlankScreen();
                for(i=0; i < 4; i++) {
                    // Display up to 4 lines of menu options
                    TextGotoXY(&textWindow, 0, i);
                    TextPrintf(&textWindow, "%s", line[offset]);
                    if (line[offset + 1] == NULL) {
                        break;
                    }
                    offset++;
                }

                // Reset the offset back to the first option displayed
                offset = offset-i;
                tmpoffset = offset;
            }
            glSetBrushType(PIXXOR);
            glBlock (0, highlight*8, 122, 8);
            glSetBrushType(PIXBLACK);
            lasthighlight = highlight;


            // Get the user's option
            waitfor((menu_option = GetKeypadOption(&offset, &highlight, menusize)));
            // Check if user selected the scrolling option

            glSetBrushType(PIXXOR);
            glBlock (0, lasthighlight*8, 122, 8);
            glSetBrushType(PIXBLACK);
            if (menu_option == -1) {

                // Set menu option to zero due to scrolling operation
                menu_option = 0;
                scrolling = TRUE;
            }
        }
    }
    tmpoffset = offset;
    return(menu_option);
}

//------------------------------------------------------------------------
// Format the Date and Time for the LCD display
//------------------------------------------------------------------------
void FormatDateTime ( void )
{
    char Day[4], Mon[4];

    ulTime = read_rtc ();			// get the RTC value
    mktm( &CurTime, ulTime );		// convert seconds to date values

    strncpy ( Day, &Days[CurTime.tm_wday*3], 3 );
    strncpy ( Mon, &Months[(CurTime.tm_mon-1)*3], 3 );
    Day[3] = 0;
    Mon[3] = 0;

    sprintf ( szTime, "%s %s %d, %d \n%d:%02d:%02d",
              Day, Mon, CurTime.tm_mday, CurTime.tm_year+1900,
              CurTime.tm_hour, CurTime.tm_min, CurTime.tm_sec );
}

//------------------------------------------------------------------------
// Date and Time prompt message routine
//------------------------------------------------------------------------
void date_prompt(char *ptr, int *col, int *row)
{
    glBlankScreen();
    TextGotoXY(&textWindow, 0, 0);
    TextPrintf(&textWindow, "%s", ptr);
    TextCursorLocation(&textWindow, col, row);

    TextGotoXY(&textWindow, 0, 3);
    TextPrintf(&textWindow, "ENTER to Continue...");
}


//------------------------------------------------------------------------
// Set Date and Time
//------------------------------------------------------------------------
void SetDateTime( void )
{
    int wKey;
    int col, row;
    char buffer[256];
    fieldupdate dateTime;

    // Setup for FAST key repeat after holding down key for 12 ticks
    keyConfig (  6,'E',0, 12, 1,  1, 1 );
    keyConfig (  2,'D',0, 12, 1,  1, 1 );
    keyConfig (  5,'+',0, 12, 1,  1, 1 );
    keyConfig (  1,'U',0, 12, 1,  1, 1 );
    keyConfig (  4,'-',0, 12, 1,  1, 1 );
    tm_rd(&CurTime);
    date_prompt("Select \n4 digit year: ", &col, &row);
    dateTime.data = (rtc.tm_year+1900);
    while(1) {
        sprintf(buffer, "%04d", dateTime.data);
        TextGotoXY(&textWindow, col, row);
        TextPrintf(&textWindow, "%s", buffer);
        while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
        if(dateTime.data < 1900 || dateTime.data > 2047) {
            dateTime.data = (1900 + rtc.tm_year);
        }
        if(wKey == 'E') {
            if( dateTime.data  >= 1900 && dateTime.data < 2048) {
                CurTime.tm_year = dateTime.data - 1900;	// offset from 1900
                break;
            }
        }
    }

    date_prompt("Enter month: ", &col, &row);
    dateTime.data = rtc.tm_mon;
    while(1) {
        sprintf(buffer, "%02d", dateTime.data);
        TextGotoXY(&textWindow, col, row);
        TextPrintf(&textWindow, "%s", buffer);
        while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
        if(wKey == 'E') {
            if( dateTime.data >= 1 && dateTime.data < 13 ) {
                CurTime.tm_mon = dateTime.data;
                break;
            }
        }
        if(dateTime.data < 1 || dateTime.data > 12) {
            dateTime.data  = (dateTime.data < 1) ? 12 : 1;
        }
    }

    date_prompt("Enter \nday of month: ", &col, &row);
    dateTime.data = rtc.tm_mday;
    while(1) {
        sprintf(buffer, "%02d", dateTime.data);
        TextGotoXY(&textWindow, col, row);
        TextPrintf(&textWindow, "%s", buffer);
        while((wKey = ProcessKeyField(NUMBER, &dateTime))== 0);
        if(wKey == 'E') {
            if( dateTime.data  >= 1 && dateTime.data < 32) {
                CurTime.tm_mday = dateTime.data;
                break;
            }
        }
        if(dateTime.data < 1 || dateTime.data > 31) {
            dateTime.data  = (dateTime.data < 1) ? 31 : 1;
        }
    }


    date_prompt("Enter \nhour (24hr): ", &col, &row);
    dateTime.data = rtc.tm_hour;
    while(1) {
        sprintf(buffer, "%02d", dateTime.data);
        TextGotoXY(&textWindow, col, row);
        TextPrintf(&textWindow, "%s", buffer);
        while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
        if(wKey == 'E') {
            if(dateTime.data >= 0 && dateTime.data < 24) {
                CurTime.tm_hour = dateTime.data;
                break;
            }
        }
        if(dateTime.data < 0 || dateTime.data > 23) {
            dateTime.data  = (dateTime.data < 0) ? 23 : 0;
        }
    }

    date_prompt("Enter minute: ", &col, &row);
    dateTime.data = rtc.tm_min;
    while(1) {
        sprintf(buffer, "%02d", dateTime.data);
        TextGotoXY(&textWindow, col, row);
        TextPrintf(&textWindow, "%s", buffer);
        while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
        if(wKey == 'E') {
            if( dateTime.data >= 0 && dateTime.data < 60) {
                CurTime.tm_min = dateTime.data;
                break;
            }
            if(wKey == 'E') {
                break;
            }
        }
        if(dateTime.data < 0 || dateTime.data > 59) {
            dateTime.data  = (dateTime.data < 0) ? 59 : 0;
        }
    }

    CurTime.tm_sec = 0;

    tm_wr(&CurTime);
    SEC_TIMER = mktime(&CurTime);


    //ulTime = mktime ( &CurTime );		// get seconds from 1/1/1980
    //write_rtc ( ulTime );				// set the real time clock
    keypadDef();

    glBlankScreen();
    while(1) {
        // Get current Date/Time
        FormatDateTime();					// convert to text

        // Display Date and Time
        glBuffLock();
        TextGotoXY(&textWindow, 0, 0);
        TextPrintf(&textWindow, "%s\n", szTime);

        // Display user exit message
        TextGotoXY(&textWindow, 0, 3);
        TextPrintf(&textWindow, "Press Key to EXIT    ");
        glBuffUnlock();

        keyProcess ();
        if((wKey = keyGet()) != 0) {
            glBlankScreen();
            break;
        }
    }
}
void DispMenu (	void	)
{
    int option, initialize;

    //------------------------------------------------------------------------
    // Initialize the controller
    //------------------------------------------------------------------------



    // Setup and center text window to be the entire display
    TextWindowFrame(&textWindow, &fi6x8, 1, 0, 121, 32);

    // Set variables to known states
    initialize = TRUE; 	// Set flag for MENU options to be displayed
    ledCntrl = LEDOFF;	// Initially disable the LED's
    //------------------------------------------------------------------------
    // Main program loop for the MENU system
    //------------------------------------------------------------------------
    for (;;) {
        costate {
            keyProcess ();
            waitfor(DelayMs(10));
        }

        costate {
            // Display the MAIN MENU
            waitfor((option = display_menu(main_menu, initialize,sizeof(main_menu))) > 0);

            // Get menu option from the user
            switch(option) {
                // Return
            case 1:
                glBlankScreen();
                cont_loop();
                // Turn Ozone ON
            case 2:
                glBlankScreen();
                ozonator_loc = 1;
                for(i=0; i < N_CHANNELS+1; i++) {
                    if(LAYER[i] == 1) {
                        nCont01[i]=1;
                        nCont02[i]=1;
                        nCont03[i]=1;
                        nCont04[i]=1;
                        nContao[i]=0;
                    }
                }
                cont_loop();
                break;

                // Turn Ozone OFF
            case 3:
                glBlankScreen();
                ozonator_loc = 0;
                cont_loop();
                break;

                // Calibrate Unit
            case 4:
                glBlankScreen();
                sector = 0;
                calibrate((int)sector);
                break;

                // Change Date/Time
            case 5:
                glBlankScreen();
                SetDateTime();
                initialize = TRUE;
                break;

                // Config settings
            case 6:
                glBlankScreen();
                SetConfig();
                break;

                // Reset
            case 7:
                exit(0);
                break;


                // User made invalid selection
            default:
                break;
            }
        }
    }
}



void SetConfig(void)
{
    int wKey;
    int col, row;
    char buffer[256];
    fieldupdate dateTime;
    save_data[0] = &settings;
    save_lens[0] = sizeof(settings);
    readUserBlockArray(save_data, save_lens, 1, 0);
    // Setup for FAST key repeat after holding down key for 12 ticks
    keyConfig (  6,'E',0, 12, 1,  1, 1 );
    keyConfig (  2,'D',0, 12, 1,  1, 1 );
    keyConfig (  5,'+',0, 12, 1,  1, 1 );
    keyConfig (  1,'U',0, 12, 1,  1, 1 );
    keyConfig (  4,'-',0, 12, 1,  1, 1 );
//Set Unit ID
    date_prompt("Set Unit ID :       ", &col, &row);
    dateTime.data = (int)settings[0];
    while(1) {
        sprintf(buffer, "%d", dateTime.data);
        TextGotoXY(&textWindow, col, row);
        TextPrintf(&textWindow, "%s    ", buffer);
        while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
        if(dateTime.data < 0 || dateTime.data > 32) {
            dateTime.data  = (dateTime.data < 0) ? 32 : 0;
        }
        if(wKey == 'E') {
            settings[0] = dateTime.data;
            printf("%d \n", settings[0]);
            break;

        }

    }

    //Set Unit  Nighttime CO2
    /*date_prompt("Set Nighttime CO2 :       ", &col, &row);
    dateTime.data = (int)settings[1];
       while(1)
    {
       sprintf(buffer, "%s", boolean[dateTime.data]);
       TextGotoXY(&textWindow, col, row);
       TextPrintf(&textWindow, "%s    ", buffer);
       while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
       if(dateTime.data < 0 || dateTime.data > 1)
       {
          dateTime.data  = (dateTime.data < 0) ? 1 : 0;
       }
       if(wKey == 'E')
       {
        settings[1] = dateTime.data;
        printf("%d \n", settings[1]);
             break;

       }

    }
    */

    //Set Layer[0] Type
    date_prompt("Set Chan[0] Layer:  ", &col, &row);
    dateTime.data = (int)settings[2];
    while(1) {
        sprintf(buffer, "%s", treatments[dateTime.data]);
        TextGotoXY(&textWindow, col, row);
        TextPrintf(&textWindow, "%s    ", buffer);
        while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
        if(dateTime.data < 0 || dateTime.data > 2) {
            dateTime.data  = (dateTime.data < 0) ? 2 : 0;
        }
        if(wKey == 'E') {
            settings[2] = dateTime.data;
            printf("%d \n", settings[2]);
            break;

        }

    }
//Set Layer[0] Setpoint
    date_prompt("Set Chan[0] Setpoint:", &col, &row);
    dateTime.data = (int)settings[3];
    printf("%d \n", dateTime.data);
    while(1) {
        sprintf(buffer, "%d", dateTime.data);
        TextGotoXY(&textWindow, col, row);
        TextPrintf(&textWindow, "%s    ", buffer);
        while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
        if(dateTime.data < 0 || dateTime.data > 1000) {
            dateTime.data  = (dateTime.data < 0) ? 1000 : 0;
        }
        if(wKey == 'E') {
            settings[3] = dateTime.data;
            printf("%d \n", settings[3]);
            break;

        }

    }
//Set Layer[0] D/A Channel
    date_prompt("Set Chan[0] D/A Chan:", &col, &row);
    dateTime.data = (int)settings[4];
    printf("%d \n", dateTime.data);
    while(1) {
        sprintf(buffer, "%d", dateTime.data);
        TextGotoXY(&textWindow, col, row);
        TextPrintf(&textWindow, "%s    ", buffer);
        while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
        if(dateTime.data < 0 || dateTime.data > 8) {
            dateTime.data  = (dateTime.data < 0) ? 8 : 0;
        }
        if(wKey == 'E') {
            settings[4] = dateTime.data;
            printf("%d \n", settings[4]);
            break;

        }

    }

//Set Layer[1] Type
    date_prompt("Set Chan[1] Layer:  ", &col, &row);
    dateTime.data = (int)settings[5];
    while(1) {
        sprintf(buffer, "%s", treatments[dateTime.data]);
        TextGotoXY(&textWindow, col, row);
        TextPrintf(&textWindow, "%s    ", buffer);
        while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
        if(dateTime.data < 0 || dateTime.data > 2) {
            dateTime.data  = (dateTime.data < 0) ? 2 : 0;
        }
        if(wKey == 'E') {
            settings[5] = dateTime.data;
            printf("%d \n", settings[5]);
            break;

        }

    }
//Set Layer[1] Setpoint
    date_prompt("Set Chan[1] Setpoint:", &col, &row);
    dateTime.data = (int)settings[6];
    printf("%d \n", dateTime.data);
    while(1) {
        sprintf(buffer, "%d", dateTime.data);
        TextGotoXY(&textWindow, col, row);
        TextPrintf(&textWindow, "%s    ", buffer);
        while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
        if(dateTime.data < 0 || dateTime.data > 1000) {
            dateTime.data  = (dateTime.data < 0) ? 1000 : 0;
        }
        if(wKey == 'E') {
            settings[6] = dateTime.data;
            printf("%d \n", settings[6]);
            break;

        }

    }
//Set Layer[1] D/A Channel
    date_prompt("Set Chan[1] D/A Chan:", &col, &row);
    dateTime.data = (int)settings[7];
    printf("%d \n", dateTime.data);
    while(1) {
        sprintf(buffer, "%d", dateTime.data);
        TextGotoXY(&textWindow, col, row);
        TextPrintf(&textWindow, "%s    ", buffer);
        while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
        if(dateTime.data < 0 || dateTime.data > 8) {
            dateTime.data  = (dateTime.data < 0) ? 8 : 0;
        }
        if(wKey == 'E') {
            settings[7] = dateTime.data;
            printf("%d \n", settings[7]);
            break;

        }

    }

//Set Anemometer Type
    date_prompt("Set Anemometer Type: ", &col, &row);
    dateTime.data = (int)settings[8];
    printf("%d \n", dateTime.data);
    while(1) {
        sprintf(buffer, "%s", anemometer[dateTime.data]);
        TextGotoXY(&textWindow, col, row);
        TextPrintf(&textWindow, "%s        ", buffer);
        while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
        if(dateTime.data < 0 || dateTime.data > 1) {
            dateTime.data  = (dateTime.data < 0) ? 1 : 0;
        }
        if(wKey == 'E') {
            settings[8] = dateTime.data;
            printf("Anemometer Type : %d \n", settings[8]);
            break;

        }

    }
    /*
        //Set Display Type
          date_prompt("Set Display Type: ", &col, &row);
       dateTime.data = (int)settings[9];
       printf("%d \n", dateTime.data);
          while(1)
       {
          sprintf(buffer, "%s", display[dateTime.data]);
          TextGotoXY(&textWindow, col, row);
          TextPrintf(&textWindow, "%s        ", buffer);
          while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
          if(dateTime.data < 0 || dateTime.data > 1)
          {
             dateTime.data  = (dateTime.data < 0) ? 1 : 0;
          }
          if(wKey == 'E')
          {
           settings[9] = dateTime.data;
           printf("Display Type : %d \n", settings[9]);
                break;

          }

       }
    */

    save_data[0] = &settings;
    save_lens[0] = sizeof(settings);
    writeUserBlockArray(0, save_data, save_lens, 1);
    printf("\nLoading...\n");
    printf("settings.[0] = %ld\n", settings[0]);
    printf("settings.[1] = %ld\n", settings[1]);
    printf("settings.[2] = %ld\n", settings[2]);
    printf("settings.[3] = %ld\n", settings[3]);
    printf("settings.[4] = %ld\n", settings[4]);
    printf("settings.[5] = %ld\n", settings[5]);
    printf("settings.[6] = %ld\n", settings[6]);
    printf("settings.[7] = %ld\n", settings[7]);
    printf("settings.[8] = %ld\n", settings[8]);
    printf("settings.[9] = %ld\n", settings[9]);

    UNIT_ID =(int)settings[0];
    nighttime_local = (int)settings[1];
    LAYER[0] = (int)settings[2];
    TARG[0] = (float)settings[3];
    //tam0509	if (LAYER[0] == 2) {TARG[0] = 0;}
    DA_Channel[0] =(int) settings[4];
    LAYER[1] = (int)settings[5];
    //tam0509  	if (LAYER[1] == 2) {TARG[1] = 0;}
    TARG[1] = (float)settings[6];
    DA_Channel[1] = (int)settings[7];
    if (!settings[8]) {
        Wind_Mult = 12.5;
        Wind_Offset = 0;
    } else {
        Wind_Mult = 10;
        Wind_Offset = 0;  //Changed 6/9/04 - Corn Rings from 2.2 to 0.0
    }
    DISPLAY = (int)settings[9];

    for (i=0; i<N_CHANNELS+1 ; i++) {
        if (LAYER[i]) {
            //Layertype 1 - Ozone Settings
            FAINT[i]=-0.00008;   //Initial=0.00002  6/24  -0.00016,-0.0008  6/25 -0.00006  8/9 -0.00008
            FAPRO[i]=-0.0016;     //Initial=0.0008 6/24  -0.0032,-0.0016
            FADIF[i]=-0.016;      //Initial=0.0080
            FCW[i]=0.5;          //Initial=0.2
            AD_GAIN[i] = 50;
            AD_OFFSET[i] = 0;
            DA_MULT[i] = 5;
        } else {
            //Layertype 0 - CO2 Settings
            FAINT[i]=-0.000015;   //
            FAPRO[i]=-0.0008;     //
            FADIF[i]=-0.016;      //
            FCW[i]=0.15;          //
            AD_GAIN[i] = 400;
            AD_OFFSET[i] = 0;
            DA_MULT[i] = 5;
        }

    }
    LAYERRELAY[0] = 6;  //Define which relay board controls each layer
    LAYERRELAY[1] = 5;


    //ulTime = mktime ( &CurTime );     // get seconds from 1/1/1980
    //write_rtc ( ulTime );          // set the real time clock
    keypadDef();

    glBlankScreen();
}

void readString_IP(void)
{
    int i;
    int location;
    auto int retval1;
    printf("%s \n",SENTENCE_IP);
    token = strtok(SENTENCE_IP,delim);

    switch(*token) { // Read Command

    case 6:  //Acknowledge Data Received on other end
        string_pos_ip = 0;
        printf("Data Acknowledged \n");
        break;

    case 65: //A Set CO2 local
        string_pos_c = 0;
        token = strtok(NULL,delim);
        if (atoi(token) == UNIT_ID) {
            token = strtok(NULL,delim);
            CO2_loc = atoi(token);
        }


        break;
    case 66: //B Set CO2 remote
        string_pos_c = 0;
        token = strtok(NULL,delim);
        CO2_rem = atoi(token);

        break;


    case 67: //C Set CO2 Nightime
        string_pos_c = 0;
        token = strtok(NULL,delim);
        nighttime_remote = atoi(token);
        token = strtok(NULL,delim);
        nighttimeCO2 = atoi(token);
        break;

    case 68: //D Download
        printf("Download \n");
        string_pos_c = 0;
        //download(RECORD);
        break;



    case 78: //N Set Ozone Remote
        string_pos_c = 0;
        token = strtok(NULL,delim);
        ozonator_loc =  atoi(token);
        break;



    case 79: //Set Ozone
        token = strtok(NULL,delim);
        ozonator_rem = atoi(token);	//set ozone
        //tam0509 if ((ora>sunr && ora<suns)  && ozonator_rem ) digOut(ChanAddr(RELAY, 0),TURNON);
        if ((ora>starttime_ozone && ora<endtime_ozone)  && ozonator_rem ) {
            digOut(ChanAddr(RELAY, 0),TURNON);
        } else {
            digOut(ChanAddr(RELAY, 0),TURNOFF);
        }

        token = strtok(NULL,delim);
        for(i=0; i < N_CHANNELS+1; i++) {

            if(LAYER[i] == 1) {
                //nCont01[i]=1;
                //nCont02[i]=1;
                //nCont03[i]=1;
                //nCont04[i]=1;
                //nContao[i]=0;
                //medo[i] = 0;
                //vento[i] = 0;
                //txlm[i] = 0;
                //nContao[i] = 0;
                //tam0509 TARG[i] = (atof(token)* OZONE_MULT);
                //tam0509 if(TARG[i] > OZONE_MAX) TARG[i] = OZONE_MAX;
            }
            fDew = 0;
        }
        string_pos_c = 0;
        break;
        // *** Added 03/27/04
    case 82:   //Reset Controller (R)
        string_pos_c = 0;
        token = strtok(NULL,delim);
        if (atoi(token) == UNIT_ID) {
            exit(0);
        }
        break;

    case 83: //Transmit second data (S)
        string_pos_c = 0;
        token = strtok(NULL,delim);
        if (atoi(token) == UNIT_ID) {
            sprintf(message,"S,%d,%02d:%02d:%02d,%.2f,%.0f,%.0f,%.2f,%.0f,%.0f,%.1f,%.0f,%d \r",UNIT_ID,rtc.tm_hour,rtc.tm_min,rtc.tm_sec,fVs[0],fpps[0],TARG[0],fVs[1],fpps[1],TARG[1],fFwind,Direz,opSect[0]);
            serCputs(message);
            while (serCwrFree() != COUTBUFSIZE) ;
        }

        break;


    case 84: //Set Time
        string_pos_c = 0;
        token = strtok(NULL,delim);
        rtc.tm_mon = atoi(token);	//set month
        token = strtok(NULL,delim);
        rtc.tm_mday = atoi(token); //set day
        token = strtok(NULL,delim);
        rtc.tm_year = (atoi(token)-1900);	//set year
        token = strtok(NULL,delim);
        rtc.tm_hour = atoi(token); //set hour
        token = strtok(NULL,delim);
        rtc.tm_min = atoi(token); //set minute
        token = strtok(NULL,delim);
        rtc.tm_sec = atoi(token); //set second

        tm_wr(&rtc);
        SEC_TIMER = mktime(&rtc);
        sprintf(message,"T,%02d/%02d/%04d,%02d:%02d:%02d \r",rtc.tm_mon,rtc.tm_mday,(rtc.tm_year+1900),rtc.tm_hour,rtc.tm_min,rtc.tm_sec);
        serDputs(message);
        while (serDwrFree() != DOUTBUFSIZE) ;
        calc_day();
        break;

    case 86: //V Reset Ozone Variables
        string_pos_c = 0;
        Reset_Ozone_Vars();
        break;

    case 87: //W Hard Reset Ozone Rings
        for(nI=0; nI<N_CHANNELS+1; nI++) { // Inizializza  contatori
            if(LAYER[nI] == 1) {
                exit(0);
            }
        }

        break;




    default:


    }
    string_pos_ip = 0;
}





