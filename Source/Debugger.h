#pragma once

#include "Defines.h"
#include "Scope.h"
#include "Value.h"
#include "VirtualMachine.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include <WS2tcpip.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Powder
{
	class POWDER_API Debugger : public VirtualMachine::DebuggerInterface
	{
	public:
		Debugger(uint16_t port);
		virtual ~Debugger();

		virtual void TrapExecution(const Executable* executable, Executor* executor) override;

		static rapidjson::Document* ReadJsonFromBuffer(const uint8_t* buffer, uint32_t& bufferSize);
		static void WriteJsonToBuffer(uint8_t* buffer, uint32_t& bufferSize, const rapidjson::Document* doc);

	private:
		static DWORD __stdcall ThreadFunc(LPVOID param);

		void ServeDebuggerClient(void);
		rapidjson::Document* ServeDebuggerClientRequest(const rapidjson::Document* requestDoc);
		
		// TODO: Store break-points, watch-points, etc. here.  Make sure we use
		//       a mutex when accessing them from either thread.

		uint16_t port;
		HANDLE threadHandle;
		SOCKET listeningSocket, clientSocket;
		uint8_t* requestBuffer;
		uint32_t requestBufferSize;
		uint8_t* responseBuffer;
		uint32_t responseBufferSize;
	};
}