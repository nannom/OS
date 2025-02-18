void main();
extern "C" void _start() { //main을 호출
    main();
    return;
}
#define KEY_ESC        (char)0x1B
#define KEY_RETURN     (char)0x0D
#define KEY_TAB        (char)0x09
#define KEY_BACKSPACE  (char)0x08
#define KEY_SPACE      (char)0x20
#define KEY_LSHIFT     (char)0xA0
#define KEY_RSHIFT     (char)0xA1
#define KEY_LCONTROL   (char)0xA2
#define KEY_RCONTROL   (char)0xA3
#define KEY_LMENU      (char)0xA4
#define KEY_RMENU      (char)0xA5
#define KEY_CAPITAL    (char)0x14
#define KEY_NUMLOCK    (char)0x90
#define KEY_SCROLL     (char)0x91
#define KEY_INSERT     (char)0x2D
#define KEY_DELETE     (char)0x2E
#define KEY_HOME       (char)0x24
#define KEY_END        (char)0x23
#define KEY_PAGEUP     (char)0x21
#define KEY_PAGEDOWN   (char)0x22
#define KEY_UP         (char)0x26
#define KEY_DOWN       (char)0x28
#define KEY_LEFT       (char)0x25
#define KEY_RIGHT      (char)0x27
#define KEY_F1         (char)0x70
#define KEY_F2         (char)0x71
#define KEY_F3         (char)0x72
#define KEY_F4         (char)0x73
#define KEY_F5         (char)0x74
#define KEY_F6         (char)0x75
#define KEY_F7         (char)0x76
#define KEY_F8         (char)0x77
#define KEY_F9         (char)0x78
#define KEY_F10        (char)0x79
#define KEY_F11        (char)0x7A
#define KEY_F12        (char)0x7B
#define KEY_PAUSE      (char)0x92
void Write_port_byte(unsigned short port, unsigned char data) {
    __asm__ __volatile__ (
        "mov dx, %0;"
        "mov al, %1;"
        "out dx, al;"
        :
        : "r" (port), "r" (data)
        : "dx", "al"
    );
}

unsigned char Read_port_byte(unsigned short port) {
    unsigned char KeyCode;

    __asm__ __volatile__ (
        "mov dx, %1;"
        "xor eax, eax;"
        "in al, dx;"
        "mov %0, al;"
        : "=r" (KeyCode)
        : "r" (port)
        : "eax", "edx"
    );

    return KeyCode;
}
class Console {
public:
    char* outputbuff;
    char* output;
    char* inputbuff;
    int inputnow = 0;
    int inbuffsize;
    int width;
    int height;
    int outbuffhight;
    volatile unsigned short cursor = 0;
    int position = 0;
    Console(int _width,int _height,int _outbuffhight,char* _outputbuff,char* _output,char* _inputbuff,int _inbuffsize) {
        width = _width;
        height = _height;
        outbuffhight = _outbuffhight;
        outputbuff = _outputbuff;
        inputbuff = _inputbuff;
        inbuffsize = _inbuffsize;
        output = _output;
        for(int i = 0;i<height * width * 2;i++) {
            *(output + i) = 0;
        }
        for(int i = 0;i<width * outbuffhight * 2;i++) {
            *(outputbuff +  i) = 0;
        }
    }
    void pushinbuff(char text) {
        if(inputnow < inbuffsize) {
            inputbuff[inputnow] = text;
            inputnow++;
        }
    }
    char popinbuff() {
        if(inputnow > 0) {
            char result = *inputbuff;
            for(int i = 0;i<inbuffsize - 1;i++) {
                inputbuff[i] = inputbuff[i + 1];
            }
            inputbuff[inbuffsize - 1] = 0;
            inputnow--;
            return result;
        }
        return 0;
    }
    void updateoutput() {
        for(int y = 0;y<height;y++) {
            for(int x = 0;x<width * 2;x++) {
                if(position + y < outbuffhight) {
                    *(output + (width * y) + x) = *(outputbuff + (width * (y + position)) + x);
                }
                else {
                    *(output + (width * y) + x) = 0;
                }
            }
        }
        Write_port_byte(0x3D4,0x0F);
        Write_port_byte(0x3D5,(unsigned char)(cursor & 0xFF));
        Write_port_byte(0x3D4,0x0E);
        Write_port_byte(0x3D5,(unsigned char)((cursor >> 8) & 0xFF));
    }
    void putoutbuff(char text) { //position변경하는거 추가해야함
        if(text == '\n') {
            cursor -= cursor % width;
            cursor += width;
        }
        else if(text == '\b') {
            cursor -= 1;
            outputbuff[cursor] = 0;
        }
        else {
            outputbuff[cursor] = text;
            outputbuff[cursor + 1] = 7;
            cursor+=2;
        }
        if(cursor / width >= outbuffhight) {
            for(int y = 1;y<outbuffhight;y++) {
                for(int x = 0; x<width * 2; x++) {
                    *(outputbuff + (width * (y - 1)) + x) = *(outputbuff + (width * y) + x);//줄 하나씩 올려서 맨 위 지우는거 마저 만들어야함
                }
            }
            for(int x = 0;x<width * 2;x++) {
                *(outputbuff + (width * (outbuffhight - 1)) + x) = 0;
            }
        }
        updateoutput();
    }
};
Console console(80,25,200,(char*)0x200000,(char*)0xB8000,(char*)(0x200000 + (80 * 25 * 2)),0x100);
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
volatile uint32_t _timer = 0;
int printf(Console _console,const char* _text,...) { //단순 출력&커서 자동 옮김
    char* arg_ptr = (char*)(&_text) + sizeof(_text);
    for(int i = 0;_text[i] != '\0';i++) {
        if(_text[i] == '%') {
            i++;
            switch (_text[i]) {
                case 'd':
                {
                    int value = *((int*)arg_ptr);
                    arg_ptr += sizeof(int);

                    int temp = value;
                    int digits = 0;
                    if (temp <= 0) {
                        digits++;
                        temp = -temp;
                    }
                    while (temp != 0) {
                        digits++;
                        temp /= 10;
                    }
                    temp = value;
                    if (value < 0) {
                        _console.putoutbuff('-');
                        temp = -temp;
                    }
                    for (int i = digits - 1; i >= 0; i--) {
                        char buffer = temp % 10 + '0';
                        temp /= 10;
                        _console.putoutbuff(buffer);
                    }
                    break;
                }
                case 's':
                {
                    char* value2 = *((char**)arg_ptr);
                    arg_ptr += sizeof(char*);
                    while(*value2 != '\0') {
                        _console.putoutbuff(*value2);
                    }
                    break;
                }
                case 'c':
                {
                    char value3 = *((char*)arg_ptr);
                    arg_ptr += sizeof(char);
                    _console.putoutbuff(value3);
                    break;
                }
                default:
                    break;
            }
        }
        else {
            _console.putoutbuff(_text[i]);
        }
    }
    return 0;
}
char issame(const char* _text1,const char* _text2) { //콘솔에서 명령어 감지할 때 사용
    while(*_text1 != 0 && *_text2 != 0) {
        if(*_text1 != *_text2) {
            return 0;
        }
        _text1+=2;
        _text2++;
    }
    return 1;
}

volatile int kbd_state[256] = {
    0,
};

struct IDT {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct IDTR {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// IDT 테이블 및 IDTR 선언
struct IDT idt[256];
struct IDTR idtr;

char scan_code_table[] = {
    (char)0, KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', KEY_RETURN,
    KEY_LCONTROL, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    KEY_LSHIFT, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KEY_RSHIFT,
    KEY_RCONTROL, KEY_LMENU, ' ', (char)0, KEY_CAPITAL,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
    KEY_F11, KEY_F12, (char)0, (char)0, (char)0, (char)0, (char)0, KEY_SCROLL, KEY_PAUSE, (char)0, (char)0, (char)0,
    KEY_INSERT, KEY_HOME, KEY_PAGEUP, (char)0, KEY_DELETE, KEY_END, KEY_PAGEDOWN, (char)0,
    KEY_UP, (char)0, KEY_LEFT, (char)0, KEY_RIGHT, (char)0, KEY_DOWN
};
char input_chars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t', 
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', ' ', KEY_RETURN // 공백 포함
};
bool iscaninput(char text) {
    for(int i = 0;i<sizeof(input_chars);i++) {
        if(text == input_chars[i]) {
            return true;
        }
    }
    return 0;
}

void idt_ignore() {
    __asm__ __volatile__(
        "cli;"
        "mov dx, 0x20;"
        "mov al, 0x20;"
        "out dx, al;"
        "sti;"
		"leave;"
		"nop;"
        "iretd;"
    );
}
void idt_timer() {
    __asm__ __volatile__(
        "cli;"
    );
    _timer++;
    __asm__ __volatile__ (
        "mov dx, 0x20;"
        "mov al, 0x20;"
        "out dx, al;"
        "sti;"
		"leave;"
		"nop;"
        "iretd;"
    );
}

void idt_keyboard() {
    __asm__ __volatile__(
        "cli;"                // 인터럽트 비활성화
    );
    //printf("test");

    unsigned char keybuf = Read_port_byte(0x60);
    if(keybuf == 0xe0) {
        keybuf = Read_port_byte(0x60);
    }
    if(keybuf & 0x80) {
        keybuf -= 0x80;
        kbd_state[scan_code_table[keybuf]] = 0;
    }
    else {
        kbd_state[scan_code_table[keybuf]] = 1;
        if(iscaninput(scan_code_table[keybuf])) {
            console.pushinbuff(scan_code_table[keybuf]);
        }
    }
    __asm__ __volatile__ (
        "mov dx, 0x20;"
        "mov al, 0x20;"
        "out dx, al;"
        "sti;"                    // 인터럽트 활성화
		"leave;"
		"nop;"
        "iretd;"                  // 인터럽트 처리 종료, 이전 상태로 복귀
    );
}

void init_pic() {
    // ICW1: 초기화 명령어 워드 1
    Write_port_byte(0x20, 0x11);  // Master PIC
    Write_port_byte(0xA0, 0x11);  // Slave PIC

    // ICW2: 인터럽트 벡터 오프셋 설정
    Write_port_byte(0x21, 0x20);  // Master PIC: 0x20 (32)
    Write_port_byte(0xA1, 0x28);  // Slave PIC: 0x28 (40)

    // ICW3: Master-Slave 연결 설정
    Write_port_byte(0x21, 0x04);  // Master PIC: Slave PIC는 IRQ2에 연결
    Write_port_byte(0xA1, 0x02);  // Slave PIC: Slave ID는 2

    // ICW4: 추가 정보
    Write_port_byte(0x21, 0x01);  // Master PIC: 8086 모드
    Write_port_byte(0xA1, 0x01);  // Slave PIC: 8086 모드

    // 인터럽트 마스크 설정
    Write_port_byte(0x21, 0x00);  // 마스터 PIC의 모든 인터럽트를 허용
    Write_port_byte(0xA1, 0x00);  // 슬레이브 PIC의 모든 인터럽트를 허용
}

// PIT 초기화 함수
void init_pit() {
    uint32_t frequency = 10000; // 100Hz (10ms 간격)
    uint16_t divisor = 1193180 / frequency;
    
    // PIT 제어 레지스터 설정
    Write_port_byte(0x43, 0x36);  // Channel 0, Low/High byte access, Mode 3

    // Divisor 설정
    Write_port_byte(0x40, divisor & 0xFF);       // Low byte
    Write_port_byte(0x40, (divisor >> 8) & 0xFF); // High byte
}

// IDT 초기화 함수
void init_idt() {

    uint64_t ptr = (uint64_t)idt_ignore;
    for(int i = 0;i<256;i++) {
        if(i != 0x21 && i != 0x20) 
        {
            idt[i].offset_low = ptr & 0xFFFF;
            idt[i].selector = 0x08;
            idt[i].zero = 0;
            idt[i].type_attr = 0x8E;
            idt[i].offset_high = (ptr >> 16) & 0xFFFF;
        }
    }

    ptr = (uint64_t)idt_timer;
    idt[0x20].offset_low = ptr & 0xFFFF;
    idt[0x20].selector = 0x08;
    idt[0x20].zero = 0;
    idt[0x20].type_attr = 0x8E;
    idt[0x20].offset_high = (ptr >> 16) & 0xFFFF;

    ptr = (uint64_t)idt_keyboard;
    idt[0x21].offset_low = ptr & 0xFFFF;
    idt[0x21].selector = 0x08;
    idt[0x21].zero = 0;
    idt[0x21].type_attr = 0x8E;
    idt[0x21].offset_high = (ptr >> 16) & 0xFFFF;
    idtr.base = (uint64_t)&idt;
    idtr.limit = sizeof(idt) - 1;
    // IDT 로드
    __asm__ __volatile__("lidt %0" : : "m"(idtr));
    __asm__ __volatile__("sti");
    printf(console,"ready");
}
int cls() {
    for(char* v = (char*)(0xB8000);v<(char*)(0xBFFFF);v++) {
        *v = 0;
    }
    console.cursor = 0;
    return 0;
}
int help() {
    printf(console,"\n<help> get commands\n<cls> clear screen\n<WaSans> WaSans!!!\n");
    return 0;
}
void main() {
    int i = 0;
    for(i = 0;i<256;i++) {
        kbd_state[i] = 0;
    }
    _timer = 0;
    for(char* v = (char*)(0xB8000);v<(char*)(0xBFFFF);v++) {
        *v++ = 0;
        *v = 0;
    }
    printf(console,"<help> to get help!\n>");
    init_pit();  // PIT 초기화
    init_pic();  // PIC 초기화
    init_idt();  // IDT 초기화
    //__asm__ __volatile__("int 0x21"); //인터럽트 테스트를 위해 실행
    int time = 10000;
    while(1) {
        /*
        if(Read_port_byte(0x64) & 1) { //키보드 입력 받는 부분
            //__asm__ __volatile__("int 0x21");
            char data = Read_port_byte(0x60);
            if(!(data & 0x80)) {
                printf("%c", scan_code_table[data]);
            }
        }
        */
        char i = console.popinbuff();
        if(i != 0) {
            //여기에 scanf구현
        }
    }
}