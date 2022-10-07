#pragma comment(lib, "PowrProf.lib")
#pragma comment (lib, "setupapi.lib")
#include <iomanip>
#include <Windows.h>
#include <iostream>
#include <powrprof.h>
#include "conio.h"
#include <batclass.h>
#include <devguid.h>
#include <Setupapi.h>
using namespace std;

int main() {
    setlocale(LC_ALL, "ru");

    while (1) {
        HDEVINFO DeviceInfoSet;
        DeviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

        SP_DEVICE_INTERFACE_DATA DeviceInterfaceData = { 0 };
        ZeroMemory(&DeviceInterfaceData, sizeof(SP_DEVINFO_DATA));
        DeviceInterfaceData.cbSize = sizeof(SP_DEVINFO_DATA);

        SetupDiEnumDeviceInterfaces(DeviceInfoSet, NULL, &GUID_DEVCLASS_BATTERY, 0, &DeviceInterfaceData);
        DWORD cbRequired = 0;

        SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, NULL, NULL, &cbRequired, NULL);
        PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, cbRequired);
        pdidd->cbSize = sizeof(*pdidd);

        SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, pdidd, cbRequired, &cbRequired, NULL);
        HANDLE hBattery = CreateFile(pdidd->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        BATTERY_QUERY_INFORMATION BatteryQueryInformation = { 0 };
        DWORD bytesWait = 0;
        DWORD bytesReturned = 0;
        DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_TAG, &bytesWait, sizeof(bytesWait), &BatteryQueryInformation.BatteryTag,
            sizeof(BatteryQueryInformation.BatteryTag), &bytesReturned, NULL) && BatteryQueryInformation.BatteryTag;
        BATTERY_INFORMATION BatteryInfo = { 0 };
        BatteryQueryInformation.InformationLevel = BatteryInformation;
        DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &BatteryQueryInformation, sizeof(BatteryQueryInformation),
            &BatteryInfo, sizeof(BatteryInfo), &bytesReturned, NULL);

        BATTERY_WAIT_STATUS bws = { 0 };
        bws.BatteryTag = BatteryQueryInformation.BatteryTag;

        BATTERY_STATUS bs;
        DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_STATUS, &bws, sizeof(bws), &bs, sizeof(bs), &bytesReturned, NULL);


        SYSTEM_POWER_STATUS sps = { 0 };

        if (!GetSystemPowerStatus(&sps)) {
            std::cout << "Ошибка в получении данных\n";
            return -1;
        }
        std::cout << "Тип питания: ";
        switch (sps.ACLineStatus) {
        case 0:
            std::cout << "Питание от батареи";
            break;
        case 1:
            std::cout << "Питание от сети";
            break;
        default:
            std::cout << "Ошибка";
        }

        std::cout << "\nРежим энергосбережения: ";
        if (sps.SystemStatusFlag) {
            std::cout << "Включен";
        }
        else {
            std::cout << "Выключен";
        }

        std::cout << "\nЗаряд аккумулятора: " << (int)sps.BatteryLifePercent << "%";

        long batteryLife = sps.BatteryLifeTime;
        int totalminutes, hours, minutes, seconds;

        if (batteryLife != -1) {
            seconds = batteryLife % 60;
            totalminutes = batteryLife / 60;
            minutes = totalminutes % 60;
            hours = totalminutes / 60;
            std::cout << "\nВремя работы ноутбука: " << hours << ":" << minutes << ":" << seconds;
        }
        else {
            std::cout << "\nВремя до полной зарядки: " << std::fixed << std::setprecision(2)
                << (double)(BatteryInfo.FullChargedCapacity - bs.Capacity) / bs.Rate * 60 << " мин\n";
        }

        std::cout << "Тип батареи:";
        for (int b = 0; b < 4; b++)
        {
            cout << BatteryInfo.Chemistry[b];
        };

        Sleep(777);

        cout << "\nВыберите режим:\n1-Спящий\n2-Гибернация\n3-Выход\n";
        char a;
        a = getchar();
        if (a == '1') {
            SetSuspendState(FALSE, FALSE, FALSE);
        }
        else if (a == '2') {
            SetSuspendState(TRUE, FALSE, FALSE);
        }
        else if (a == '3') break;
        Sleep(777);
        system("cls");
    }

    return 0;
}