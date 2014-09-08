/* a simple .tbx editor in C++ */
#include <vector>
#include <iostream>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
using namespace std;
struct command_t {
    unsigned char cmdbyte;
    uint32_t freq;
    uint32_t time;
};
vector<command_t> song;
void read_tbx(const char* file)
{
    unsigned char buf[4];
    int fd=open(file, O_RDONLY);
    int ret=read(fd, buf, 2);
    if(ret!=2 || buf[0]!=0xf3 || buf[1]!=0x76)
    {
        cout << "File is does not exist/is corrupted." << endl;
        exit(1);
    }
    do {
        command_t cmd;
        ret=0;
        ret+=read(fd, &(cmd.cmdbyte), 1);
        ret+=read(fd, buf, 4);
        cmd.freq=(buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
        ret+=read(fd, buf, 4);
        cmd.time=(buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
        if(ret==9)
            song.push_back(cmd);
    } while(ret==9);
    close(fd);
}
void print_tbx(void)
{
    for(int i=0;i<song.size();++i)
    {
        switch(song[i].cmdbyte)
        {
        case 0x00:
            cout << i << ": " << song[i].freq << " microseconds silence" << endl;
            break;
        case 0xff:
            cout << i << ": " << song[i].freq << " Hz tone for " << song[i].time << " microseconds" << endl;
            break;
        default:
            cout << "Unknown" << endl;
        }
    }
}
void write_tbx(const char* file)
{
    int fd=open(file, O_TRUNC | O_WRONLY);
    char buf[2]={0xf3, 0x76};
    write(fd, buf, 2);
    for(int i=0;i<song.size();++i)
    {
        write(fd, &(song[i].cmdbyte), 1);
        unsigned char buf[8];
        buf[0]=(song[i].freq&0xFF000000)>>24;
        buf[1]=(song[i].freq&0xFF0000)>>16;
        buf[2]=(song[i].freq&0xFF00)>>8;
        buf[3]=(song[i].freq&0xFF)>>0;
        buf[4]=(song[i].time&0xFF000000)>>24;
        buf[5]=(song[i].time&0xFF0000)>>16;
        buf[6]=(song[i].time&0xFF00)>>8;
        buf[7]=(song[i].time&0xFF)>>0;
        write(fd, buf, 8);
    }
    close(fd);
}
int main(int argc, const char* argv[])
{
    if(argc!=2)
    {
        cout << "Usage: " << argv[0] << " [FILE]" << endl << "Edit/create TBX file" << endl;
    }

    read_tbx(argv[1]);
    cout << "Commands:" << endl <<
        "t - add tone" << endl <<
        "s - add silence" << endl <<
        "m - modify tone/silence" << endl <<
        "d - delete tone/silence" << endl <<
        "e - write+exit" << endl;
    char command=0;
    while(command!='e')
    {
        print_tbx();
        cout << "> ";
        cin.get(command);
        cin.clear();
        switch(command)
        {
        case 't':
        {
            command_t note;
            note.cmdbyte=0xff;
            cout << "Enter frequency: ";
            cin >> note.freq;
            cout << "Enter duration (microseconds): ";
            cin >> note.time;
            song.push_back(note);
            break;
        }
        case 's':
        {
            command_t silence;
            silence.cmdbyte=0;
            cout << "Enter duration (microseconds): ";
            cin >> silence.freq;
            silence.time=0xDEADBEEF;
            song.push_back(silence);
            break;
        }
        case 'm':
        {
            int idx;
            cout << "Enter index of note: ";
            cin >> idx;
            if(idx<0 || idx>=song.size())
            {
                cout << "Bad index!" << endl;
                break;
            }
            cout << "Note " << idx << " is ";
            if(song[idx].cmdbyte==0xff)
            {
                cout << "tone at " << song[idx].freq << " Hz for " << song[idx].time << " microseconds." << endl;
            }
            else if(song[idx].cmdbyte==0x00)
            {
                cout << "silence for " << song[idx].freq << " microseconds." << endl;
            }
            command_t newnote;
            cout << "Enter new type (s or t): ";
            char newtype;
            cin >> newtype;
            if(newtype=='s')
            {
                newnote.cmdbyte=0;
                cout << "Enter duration (in microseconds): ";
                cin >> newnote.freq;
                newnote.time=0xDEADBEEF;
            }
            else
            {
                newnote.cmdbyte=0xff;
                if(newtype!='t')
                {
                    cout << "defaulting to t." << endl;
                }
                cout << "Enter frequency: ";
                cin >> newnote.freq;
                cout << "Enter duration (in microseconds): ";
                cin >> newnote.time;
            }
            song[idx]=newnote;
            break;
        }
        case 'd':
        {
            cout << "Enter index of note to be removed: ";
            int idx;
            cin >> idx;
            if(idx<0 || idx>=song.size())
            {
                cout << "Bad index!" << endl;
                break;
            }
            song.erase(song.begin()+idx);
            break;
        }
        case 'e':
            break;
        default:
            cout << "Unknown." << endl;
            break;
        }
    }
    cout << "Writing..." << endl;
    write_tbx(argv[1]);
}
