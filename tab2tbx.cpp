#include <vector>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <stdint.h>
#include <fcntl.h>
using namespace std;

const int string_freqs[6][27]={
    /* 1st string */
    {
        330, /* open */
        349,
        370,
        392,
        415,
        440,
        466,
        494,
        523,
        554,
        587,
        622,
        659,
        698,
        740,
        784,
        830,
        880,
        932,
        988,
        1047,
        1109,
        1175,
        1245,
        1319,
        1397,
        1480
    },
    /* 2nd string */
    {
        247, /* open */
        262,
        277,
        294,
        311,
        330,
        349,
        370,
        392,
        415,
        440,
        466,
        494,
        523,
        554,
        587,
        622,
        659,
        698,
        740,
        784,
        830,
        880,
        932,
        988,
        1047,
        1109
    },
    /* third string */
    {
        196,
        208,
        220,
        233,
        247,
        262,
        277,
        294,
        311,
        330,
        349,
        370,
        392,
        415,
        440,
        466,
        494,
        523,
        554,
        587,
        622,
        659,
        698,
        740,
        784,
        830,
        880
    },
    /* fourth string */
    {
        147,
        156,
        165,
        175,
        185,
        196,
        208,
        220,
        233,
        247,
        262,
        277,
        294,
        311,
        330,
        349,
        370,
        392,
        415,
        440,
        466,
        494,
        523,
        554,
        587,
        622,
        659
    },
    /* fifth string */
    {
        110,
        117,
        123,
        131,
        139,
        147,
        156,
        165,
        175,
        185,
        196,
        208,
        220,
        233,
        247,
        262,
        277,
        294,
        311,
        330,
        349,
        370,
        392,
        415,
        440,
        466,
        494
    },
    /* sixth string */
    {
        82,
        87,
        93,
        98,
        104,
        110,
        117,
        123,
        131,
        139,
        147,
        156,
        165,
        175,
        185,
        196,
        208,
        220,
        233,
        247,
        262,
        277,
        294,
        311,
        330,
        349,
        370
    }
};
enum { NOTE_NONE, NOTE };

struct note_t {
    int type;
    int fret; /* guitar fret */
};
struct tbx_note_t {
    unsigned char cmdbyte;
    uint32_t freq;
    uint32_t time; /* microsecs */
};
int main(int argc, const char* argv[])
{
    cout << "Reading..." << endl;
    string tab_lines[6];
    for(int i=0;i<6;++i)
    {
        getline(cin, tab_lines[i]);
    }
    cout << "Reading into tab" << endl;
    vector<struct note_t> tab[6];
    for(int i=0;i<6;++i)
    {
        stringstream ss(tab_lines[i]);
        while(ss)
        {
            struct note_t n;
            string str;
            ss >> str;
            if(str=="--")
                n.type=NOTE_NONE;
            else
                n.type=NOTE;
            if(n.type==NOTE)
            {
                n.fret=atoi(str.c_str());
            }
            tab[i].push_back(n);
        }
    }

    for(int i=0;i<6;++i)
    {
        for(int j=0;j<tab[i].size();++j)
        {
            if(tab[i][j].type==NOTE)
            {
                cout << tab[i][j].fret;
            }
            else
                cout << " ";
        }
        cout << endl;
    }
    cout << "Writing..." << endl;
    int fd=open("tabout.tbx", O_CREAT|O_WRONLY, 0644);
    /* now write, frame-by-frame */
    if(fd<0)
    {
        cerr << "Error: file exists?" << endl;
        return 1;
    }
    const unsigned char header[2] = { 0xf3, 0x76 };
    write(fd, header, 2);
#define NOTE_DURATION 500000
#define SILENCE_DURATION 125000
    for(int i=0;i<tab[0].size();++i)
    {
        cout << "Writing one frame..." << endl;
        struct tbx_note_t n;
        n.cmdbyte=0;
        n.time=NOTE_DURATION;
        n.freq=0;
        struct note_t frame[6]; /* 0=1st string, 5=6th string */
        cout << "Reading frame..." << endl;
        for(int j=0;j<6;++j)
        {
            frame[j]=tab[j][i];
            if(frame[j].type==NOTE)
            {
                n.cmdbyte=0xff;
            }
        }
        /* we now have the frame in frame[] and the command byte set. */
        uint32_t avg=0;
        int num_notes=0;
        /* find the sum of the played notes */
        for(int j=0;j<6;++j)
        {
            if(frame[j].type==NOTE)
            {
                avg+=string_freqs[j][frame[j].fret];
                ++num_notes;
            }
        }
        if(num_notes)
            avg/=1.00/num_notes;
        else
            avg=NOTE_DURATION;
        n.freq=avg;
        cout << "Average freq for frame is " << avg << endl;
        unsigned char buf[9];
        buf[0]=n.cmdbyte;
        buf[1]=(n.freq&0xFF000000)>>24;
        buf[2]=(n.freq&0xFF0000)>>16;
        buf[3]=(n.freq&0xFF00)>>8;
        buf[4]=(n.freq&0xFF)>>0;
        buf[5]=(n.time&0xFF000000)>>24;
        buf[6]=(n.time&0xFF0000)>>16;
        buf[7]=(n.time&0xFF00)>>8;
        buf[8]=(n.time&0xFF)>>0;
        write(fd, buf, 9);

        /* now write the pause in betweent the notes */
        n.cmdbyte=0;
        n.freq=SILENCE_DURATION;
        n.time=0;
        buf[0]=n.cmdbyte;
        buf[1]=(n.freq&0xFF000000)>>24;
        buf[2]=(n.freq&0xFF0000)>>16;
        buf[3]=(n.freq&0xFF00)>>8;
        buf[4]=(n.freq&0xFF)>>0;
        buf[5]=(n.time&0xFF000000)>>24;
        buf[6]=(n.time&0xFF0000)>>16;
        buf[7]=(n.time&0xFF00)>>8;
        buf[8]=(n.time&0xFF)>>0;
        write(fd, buf, 9);
    }
    close(fd);
}
