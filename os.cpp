void main();
extern "C" void _start() { //main을 호출
    main();
    return;
}
#define WIDTH 320
#define HEIGHT 200

void Write_port_byte(unsigned short port, unsigned char data) {
    __asm__ __volatile__ (
        "cli;"
        "pusha;"
        "mov dx, %0;"
        "mov al, %1;"
        "out dx, al;"
        "popa;"
        "sti;"
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

typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
volatile char *video = (char*)(0xB8000);
volatile unsigned short position = 0;
volatile uint32_t _timer = 0;
int printf(const char* _text,...) { //단순 출력&커서 자동 옮김
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
                        *video++ = '-';
                        *video++ = 7;
                        temp = -temp;
                    }
                    for (int i = digits - 1; i >= 0; i--) {
                        char buffer = temp % 10 + '0';
                        temp /= 10;
                        *video++ = buffer;
                        *video++ = 7;
                        position++;
                    }
                    break;
                }
                case 's':
                {
                    char* value2 = *((char**)arg_ptr);
                    arg_ptr += sizeof(char*);
                    while(*value2 != '\0') {
                        if(*value2 == '\n') {
                            int d = (int)video - 0xB8000;
                            d -= d % 160;
                            d += 0xB8000 + 160;
                            video = (char*)d;
                            position += (short)80 - (short)(position % 80);
                        }
                        else {
                            *video++ = *value2;
                            *video++ = 7;
                            value2++;
                            position++;
                        }
                    }
                    break;
                }
                case 'c':
                {
                    char value3 = *((char*)arg_ptr);
                    arg_ptr += sizeof(char);
                    *video++ = value3;
                    *video++ = 7;
                    position++;
                    break;
                }
                default:
                    break;
            }
        }
        else {
            if(_text[i] == '\n') {
                int d = (int)video - 0xB8000;
                d -= d % 160;
                d += 0xB8000 + 160;
                video = (char*)d;
                position += (short)80 - (short)(position % 80);
            }
            else {
                *video++ = _text[i];
                *video++ = 7;
                position++;
            }
        }
    }
    Write_port_byte(0x3D4,0x0F);
    Write_port_byte(0x3D5,(unsigned char)(position & 0xFF));
    Write_port_byte(0x3D4,0x0E);
    Write_port_byte(0x3D5,(unsigned char)((position >> 8) & 0xFF));
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
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '};
void idt_ignore() {
    __asm__ __volatile__(
        "cli;"
        "pusha;"
        "mov dx, 0x20;"
        "mov al, 0x20;"
        "out dx, al;"
        "mov dx, 0xA0;"
        "out dx, al;"
        "popa;"
        "sti;"
        "iretd;"
    );
}

void idt_timer() {
    __asm__ __volatile__(
        "cli;"
        "pusha;"
    );
    _timer++;
    __asm__ __volatile__ (
        "mov dx, 0x20;"
        "mov al, 0x20;"
        "out dx, al;"
        "popa;"
        "sti;"
        "iretd;"
    );
}

void idt_keyboard() {
    __asm__ __volatile__(
        "cli;"                // 인터럽트 비활성화
        "pusha;"              // 모든 레지스터 저장
    );
    printf("test");

    unsigned char keybuf = Read_port_byte(0x60);
    printf("%c", scan_code_table[keybuf]);
    __asm__ __volatile__ (
        "mov dx, 0x20;"
        "mov al, 0x20;"
        "out dx, al;"
        "popa;"                   // 모든 레지스터 복원
        "sti;"                    // 인터럽트 활성화
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

    uint32_t ptr = (uint32_t)idt_ignore;
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

    ptr = (uint32_t)idt_timer;
    idt[0x20].offset_low = ptr & 0xFFFF;
    idt[0x20].selector = 0x08;
    idt[0x20].zero = 0;
    idt[0x20].type_attr = 0x8E;
    idt[0x20].offset_high = (ptr >> 16) & 0xFFFF;

    ptr = (uint32_t)idt_keyboard;
    idt[0x21].offset_low = ptr & 0xFFFF;
    idt[0x21].selector = 0x08;
    idt[0x21].zero = 0;
    idt[0x21].type_attr = 0x8E;
    idt[0x21].offset_high = (ptr >> 16) & 0xFFFF;
    idtr.base = (uint32_t)&idt;
    idtr.limit = sizeof(idt) - 1;
    // IDT 로드
    __asm__ __volatile__("lidt %0" : : "m"(idtr));
    __asm__ __volatile__("sti");
    printf("ready");
}
int cls() {
    for(char* v = (char*)(0xB8000);v<(char*)(0xBFFFF);v++) {
        *v = 0;
    }
    video = (char*)0xB8000;
    return 0;
}
int help() {
    printf("\n<help> get commands\n<cls> clear screen\n<WaSans> WaSans!!!\n");
    return 0;
}
void main() {
    int i = 0;
    for(i = 0;i<256;i++) {
        kbd_state[i] = 0;
    }
    video = (char*)(0xB8000);
    _timer = 0;
    for(char* v = (char*)(0xB8000);v<(char*)(0xBFFFF);v++) {
        *v++ = 0;
        *v = 0;
    }
    printf("<help> to get help!\n>");
    //init_pit();  // PIT 초기화
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
        for(i = 0;i<128 && 0;i++) { //콘솔 관련(미사용중)
            if(kbd_state[i] == 1) {
                if(scan_code_table[i] == '\n') {
                    int d = (int)video - 0xB8000;
                    d -= d % 160;
                    d += 0xB8000;
                    if(issame(((char*)d)+2,"help") == 1) {
                        help();
                    }
                    else if(issame(((char*)d) + 2, "cls") == 1) {
                        cls();
                    }
                    else {
                        printf("\nunknowing command");
                    }
                    printf("\n>");
                }
                else if(scan_code_table[i] == '\b') {
                    if(video > (char*)0xB8000) {
                        video -= 2;
                        position--;
                        *video = 0;
                        if(*(video-2) == '\0') {
                            while(video > (char*)0xB8000) {
                                video -= 2;
                                position--;
                                *video = 0;
                                if(*(video - 2) != '\0') {
                                    break;
                                }
                            }
                        }
                    }
                    Write_port_byte(0x3D4,0x0F);
                    Write_port_byte(0x3D5,(unsigned char)(position & 0xFF));
                    Write_port_byte(0x3D4,0x0E);
                    Write_port_byte(0x3D5,(unsigned char)((position >> 8) & 0xFF));
                }
                else if(i == 42 || i == 54) {
                }
                else {
                    if(scan_code_table[i] == 0xFF) {
                        //printf("%c", hx[i >> 4]);
                        //printf("%c", hx[i & 0xF]);
                    }
                    else {
                        if(kbd_state[42] == 0 && kbd_state[54] == 0) {
                            printf("%c", scan_code_table[i]);
                        }
                        else {
                            printf("%c", scan_code_table[i]);
                        }
                    }
                }
                kbd_state[i] = 2;
            }
        }
        //printf("%d\n", _timer);
        //printf("abcde");
        if(video >= (char*)(0xB8FA0)) {
            video = (char*)(0xB8000);
        }
    }
}
