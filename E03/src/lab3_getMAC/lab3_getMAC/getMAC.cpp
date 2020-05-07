#include <WinSock2.h>
#include <windows.h>
#include <Iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "WS2_32.lib") 
using namespace std;

int main()
{
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	ULONG ulLen = 0;
	::GetAdaptersInfo(pAdapterInfo, &ulLen);
	pAdapterInfo = (PIP_ADAPTER_INFO)::malloc(ulLen);
	::GetAdaptersInfo(pAdapterInfo, &ulLen);
	int count = 0;
	while (pAdapterInfo) {
		printf("NIC %d: \n", ++count);
		printf("\tIP: %s; Mask: %s; Gateway: %s\n", pAdapterInfo->IpAddressList.IpAddress.String, pAdapterInfo->IpAddressList.IpMask.String, pAdapterInfo->GatewayList.IpAddress.String);
		printf("\tName: %s; Desc: %s\n", pAdapterInfo->AdapterName, pAdapterInfo->Description);
		printf("\tMAC: ");
		for (size_t i = 0; i < pAdapterInfo->AddressLength; i++)
		{
			printf("%02X", pAdapterInfo->Address[i]);
		} printf("\n");
		pAdapterInfo = pAdapterInfo->Next;
	}
	system("pause");
	if (pAdapterInfo) { free(pAdapterInfo); }
}