#include <iostream>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
using namespace std;
int main(int argc, const char *argv[])
{
    if(argc!=2)
    {
        cout << "Usage: " << argv[0] << " FILE" << endl << "Read GRUB_INIT_TUNE data from STDIN and write .tbx conversion to FILE" << endl;
        return 1;
    }
    int fd=open(argv[1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
    unsigned char buf[2]={0xf3, 0x76};
    write(fd, buf, 2);
    int tempo;
    cin >> tempo;
    cout << "Tempo is " << tempo << "BPM." << endl;
    int multiplier=60.0/tempo*1000000.0;
    while(cin)
    {
        unsigned char buf[9];
        buf[0]=0xff;
        uint32_t freq, dur;
        cin >> freq >> dur;
        buf[1]=(freq&0xFF000000)>>24;
        buf[2]=(freq&0xFF0000)>>16;
        buf[3]=(freq&0xFF00)>>8;
        buf[4]=(freq&0xFF)>>0;
        dur*=multiplier;
        buf[5]=(dur&0xFF000000)>>24;
        buf[6]=(dur&0xFF0000)>>16;
        buf[7]=(dur&0xFF00)>>8;
        buf[8]=(dur&0xFF)>>0;
        write(fd, buf, 9);
        buf[0]=0x00;
        buf[1]=(multiplier&0xFF000000)>>24;
        buf[2]=(multiplier&0xFF0000)>>16;
        buf[3]=(multiplier&0xFF00)>>8;
        buf[4]=(multiplier&0xFF)>>0;
        buf[5]=0xDE;
        buf[6]=0xAD;
        buf[7]=0xBE;
        buf[8]=0xEF;
        write(fd, buf, 9);
    }
}
