#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;

int main()
{
    char c[1024];
    for (int i = 0; i <= 1023; i++)
    {
        c[i] = 'a';
    }
    int cir;
    cout << "��������Ҫ���ļ���С��KiB����" << endl;
    cin >> cir;
    fstream fs;
    fs.open("editor.dat", ios::out);
    for (int i = 1; i <= cir; i++)
    {
        fs.write(c, 1024);
    }
    fs.close();
    cout << "���ɳɹ���" << endl;
    system("pause");
    return 0;
}
