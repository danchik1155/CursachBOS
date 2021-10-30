
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <conio.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <winsock2.h>
#pragma comment (lib,"Ws2_32.lib")



#define MAX_PACKET_SIZE    0x10000
#define SIO_RCVALL         0x98000001
// Буфер для приёма данных
char Buffer[MAX_PACKET_SIZE]; // 64 Kb

using namespace std;

//Структура заголовка IP-пакета

typedef struct IPHeader {
	UCHAR   iph_verlen;   // версия и длина заголовка
	UCHAR   iph_tos;      // тип сервиса
	USHORT  iph_length;   // длина всего пакета
	USHORT  iph_id;       // Идентификация
	USHORT  iph_offset;   // флаги и смещения
	UCHAR   iph_ttl;      // время жизни пакета
	UCHAR   iph_protocol; // протокол
	USHORT  iph_xsum;     // контрольная сумма
	ULONG   iph_src;      // IP-адрес отправителя
	ULONG   iph_dest;     // IP-адрес назначения
	USHORT params;		// параметры (до 320 бит)
	UCHAR data;			// данные (до 65535 октетов)
} IPHeader;

char src[10];
char dest[10];
char ds[15];
char dso[5];
unsigned short lowbyte;
unsigned short hibyte;
using namespace std;

int main()
{
	//1.Инициализация Winsock
	WSADATA     wsadata;	// Cодержит информацию о проинициализированной версии WinsocAPI
	SOCKET      s;			// Cлущающий сокет.
	char        name[128];	// Имя хоста (компьютера).
	HOSTENT*	phe;		// Информация о хосте.
	SOCKADDR_IN sa;			// Адрес хоста
	IN_ADDR		sa1;		// Информация о подключении
	unsigned long flag = 1;	// Флаг PROMISC Вкл/выкл.
	int			error = 1;	// Код ошибки

	//MAKEWORD( 2, 2 ) - связывает версию. Версия 2.2
	//WSAStartup() функция которая инициализирует Winsock

	if (FAILED(WSAStartup(MAKEWORD(2, 2), &wsadata)))
	{
		// Error...
		error = WSAGetLastError();
		cout << "error1: " << error << endl;
		exit(1);
	}


	//2.Создание сокета
	//AF_INET- (address family) протокол семейства интернет
	//SOCK_STREAM (надёжная потокоориентированная служба (сервис) или потоковый сокет для tcp)
	//SOCK_DGRAM (служба датаграмм или датаграммный сокет)
	//SOCK_RAW (cырой сокет) сырой протокол поверх сетевого уровня).
	//IPPROTO_IP
	//IPPROTO_UDP-udp протокол
	//IPPROTO_TCP-тсp протокол

	if (INVALID_SOCKET == (s = socket(AF_INET, SOCK_RAW, IPPROTO_IP)))//создаем дескриптор сокета
	{
		error = WSAGetLastError();
		cout << "error2: " << error << endl;
		exit(1);
	}
	
	gethostname(name, sizeof(name));
	cout << "Hostname: " << name << endl;
	phe = gethostbyname(name);
	ZeroMemory(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	for (int i = 0; i < 5; i++)
		cout << i << " " << ((struct in_addr*)phe->h_addr_list[i]) << endl;
	sa.sin_addr.s_addr = ((struct in_addr*)phe->h_addr_list[1])->s_addr;

	// bind устанавливает соединение сокета с адресом и принимает 3 параметра: 
	// sockfd (s) — дескриптор, представляющий сокет при привязке
	// serv_addr (sa) — указатель на структуру sockaddr, представляющую адрес, к которому привязываем.
	// addrlen — поле socklen_t, представляющее длину структуры sockaddr.
	//
	bind(s, (SOCKADDR*)&sa, sizeof(SOCKADDR));

	// Включение promiscuous mode. Код управления SIO_RCVALL позволяет сокету принимать все пакеты IPv4 или IPv6, проходящие через сетевой интерфейс.
	ioctlsocket(s, SIO_RCVALL, &flag);

	// Бесконечный цикл приёма IP-пакетов.
	while (true)
	{
		int count;
		count = recv(s, Buffer, sizeof(Buffer), 0);
		// обработка IP-пакета
		if (count >= sizeof(IPHeader))
		{
			IPHeader* hdr = (IPHeader*)Buffer;
			//Начинаем разбор пакета...

			strcpy(src, "Пакет: ");
			CharToOem(src, dest);
			printf(dest);
			// Преобразуем в понятный вид адрес отправителя.
			printf("From ");
			sa1.s_addr = hdr->iph_src;
			printf(inet_ntoa(sa1));

			// Преобразуем в понятный вид адрес получателя.
			printf(" To ");
			sa1.s_addr = hdr->iph_dest;
			printf(inet_ntoa(sa1));

			// Вычисляем протокол. Полный список этих констант
			// содержится в файле winsock2.h
			printf(" Prot: ");
			if (hdr->iph_protocol == IPPROTO_TCP) printf("TCP "); else
				if (hdr->iph_protocol == IPPROTO_UDP) printf("UDP "); else
					printf("UNKNOWN ");

			// Вычисляем размер. Так как в сети принят прямой порядок
			// байтов, а не обратный, то прийдётся поменять байты местами.
			printf("Size: ");
			lowbyte = hdr->iph_length >> 8;
			hibyte = hdr->iph_length << 8;
			hibyte = hibyte + lowbyte;
			printf("%s", _itoa(hibyte, ds, 10));

			// Вычисляем время жизни пакета.
			printf("%s", _itoa(hibyte, ds, 10));
			printf(" TTL:%s", _itoa(hdr->iph_ttl, ds, 10));

			cout << endl;
		}
	}

	closesocket(s);
	WSACleanup();
}