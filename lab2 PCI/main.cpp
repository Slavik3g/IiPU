#include "hexioctrl.h"
#include "pci_codes.h"
#include <conio.h>
#include <stdlib.h>

void showDeviceInfo(int, int, int);
void decodeNames(unsigned long, unsigned long);
unsigned long calculateAddress(int, int, int, int);
unsigned long getRegData(unsigned long);

int main()
{
	ALLOW_IO_OPERATIONS;

	// Перебор всех возможных вариантов адресов устройств
	for(int bus = 0; bus < 256; bus++)	          // Номер шины, 8 бит, 2^8 = 256
		for(int dev = 0; dev < 32; dev++)		  // Номер устройства, 5 бит, 2^5 = 32
			for(int func = 0; func < 8; func++)	  // Номер функции, 3 бита, 2^3 = 8
				showDeviceInfo(bus, dev, func);		
	
	printf("\n\n");
	system("pause");
	return 0;
}

void showDeviceInfo(int bus, int dev, int func)
{
	unsigned long configAddress = calculateAddress(bus, dev, func, 0x00); // Получить адрес регистра  для вызова конфиг. цикла
	unsigned long RegData = getRegData(configAddress);   

	if (RegData == -1) // Если устройства с данным адресом не существует
		return;
	
	// Получение Device ID и Vendor ID из регистра 0x00
	unsigned long DeviceID = RegData >> 16;               // Сдвиг вправо на 16 бит (Device ID) 
	unsigned long VendorID = RegData - (DeviceID << 16);


	// Вывод на экран
	printf("PCI bus: %x, device: %x, function: %x\n", bus, dev, func); 
	printf("Device ID: %x, Vendor ID: %x\n", DeviceID, VendorID);
		
	// Вывести на экран текстовые строки наименований производителей, устройств и классов
	decodeNames(DeviceID, VendorID);
	printf("--------------------------------------------------------------------------\n\n");	
}

// Получить содержимое регистра по адресу configAddress
unsigned long getRegData(unsigned long configAddress)
{
	unsigned long regData;
	__asm
	{
		mov eax, configAddress
		mov dx, 0CF8h
        out dx, eax       // Записать в порт 0CF8h (CONFIG_ADDRESS) адрес регистра устройства
        mov dx, 0CFCh
        in eax, dx        // Прочитать содержимое регистра устройства из порта 0CFCh (CONFIG_DATA)
		mov regData, eax
	}
	return regData;
}

// Составить адрес конфигурационного регистра по номеру шины, 
// номеру устройства, номеру функции и номеру регистра
unsigned long calculateAddress(int bus, int device, int function, int reg)
{
	unsigned long address = 1;
	address = address << 15;
	address += bus;          // Номер шины, 8 бит
	address = address << 5;      
	address += device;       // Номер устройства, 5 бит
	address = address << 3;
	address += function;     // Номер функции, 3 бита
	address = address << 8;
	address += reg;
	return address;
}


// Расшифровка наименований производителей, устройств и классов
void decodeNames(unsigned long DeviceId, unsigned long VendorId)
{
	// Название и описание устройства
	for(int i = 0; i < PCI_DEVTABLE_LEN; i++)
	{
		if(PciDevTable[i].VenId == VendorId && PciDevTable[i].DevId == DeviceId)		
			printf("%s, %s\n", PciDevTable[i].Chip, PciDevTable[i].ChipDesc);		
	}

	// Название производителя устройства
	for(int i = 0; i < PCI_VENTABLE_LEN; i++)
	{
		if(PciVenTable[i].VenId == VendorId)		
			printf("%s\n", PciVenTable[i].VenFull);		
	}
}