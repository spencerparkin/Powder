#include "Debugger.h"
#include "rapidjson/cursorstreamwrapper.h"
#include "rapidjson/writer.h"
#include <assert.h>

namespace Powder
{
	Debugger::Debugger(uint16_t port)
	{
		this->requestBufferSize = 10 * 1024;
		this->requestBuffer = new uint8_t[this->requestBufferSize];
		this->responseBufferSize = 10 * 1024;
		this->responseBuffer = new uint8_t[this->responseBufferSize];
		this->port = port;
		this->listeningSocket = INVALID_SOCKET;
		this->clientSocket = INVALID_SOCKET;
		this->threadHandle = ::CreateThread(nullptr, 0, &Debugger::ThreadFunc, this, 0, nullptr);
	}

	/*virtual*/ Debugger::~Debugger()
	{
		if (this->clientSocket != INVALID_SOCKET)
			::closesocket(this->clientSocket);

		if (this->listeningSocket != INVALID_SOCKET)
			::closesocket(this->listeningSocket);

		::WaitForSingleObject(this->threadHandle, INFINITE);

		delete[] this->requestBuffer;
		delete[] this->responseBuffer;
	}

	/*virtual*/ void Debugger::TrapExecution(const Executable* executable, Executor* executor)
	{
		//...
	}

	/*static*/ DWORD Debugger::ThreadFunc(LPVOID param)
	{
		Debugger* debugger = (Debugger*)param;
		debugger->ServeDebuggerClient();
		return 0;
	}

	void Debugger::ServeDebuggerClient(void)
	{
		WSADATA data;
		int result = ::WSAStartup(MAKEWORD(2, 2), &data);
		assert(result != 0);

		this->listeningSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		assert(this->listeningSocket != INVALID_SOCKET);

		sockaddr_in addr;
		addr.sin_family = AF_INET;
		::InetPtonA(addr.sin_family, "127.0.0.1", &addr.sin_addr);
		addr.sin_port = ::htons(this->port);

		result = ::bind(this->listeningSocket, (sockaddr*)&addr, sizeof(addr));
		assert(result == 0);

		result = ::listen(this->listeningSocket, 1);
		assert(result == 0);

		while (true)
		{
			// Block here until a client connects.
			this->clientSocket = ::accept(this->listeningSocket, nullptr, nullptr);

			// We may get an invalid socket if we're being signaled to exit the thread.
			if (this->clientSocket == INVALID_SOCKET)
				break;

			// Enter the service loop.  All we do here is simply send a response for every request.
			uint32_t i = 0;
			while (true)
			{
				uint32_t bytesReceived = ::recv(this->clientSocket, (char*)&this->requestBuffer[i], this->requestBufferSize - i, 0);
				
				// Again, this may be a signal for us to exit the thread.  It could also be that the client disconnected.
				if (bytesReceived == SOCKET_ERROR)
					break;

				i += bytesReceived;
				rapidjson::Document* requestDoc = this->ReadJsonFromBuffer(this->requestBuffer, i);
				if (requestDoc)
				{
					rapidjson::Document* responseDoc = this->ServeDebuggerClientRequest(requestDoc);
					assert(responseDoc != nullptr);
					delete requestDoc;
					uint32_t j = this->responseBufferSize;
					this->WriteJsonToBuffer(this->responseBuffer, j, responseDoc);
					delete responseDoc;
					uint32_t bytesSent = ::send(this->clientSocket, (char*)this->responseBuffer, j, 0);
					assert(bytesSent == j);
					j = 0;
					while (i < this->requestBufferSize)		// This could be sped up by moving only what needs to be moved.
						this->requestBuffer[j++] = this->requestBuffer[i++];
					i = j;
				}
				else if(i >= this->requestBufferSize)
				{
					uint8_t* newBuffer = new uint8_t[this->requestBufferSize * 2];
					::memcpy(newBuffer, this->requestBuffer, this->requestBufferSize);
					delete[] this->requestBuffer;
					this->requestBuffer = newBuffer;
					this->requestBufferSize *= 2;
				}
			}

			::closesocket(this->clientSocket);
			this->clientSocket = INVALID_SOCKET;
		}

		::WSACleanup();
	}

	rapidjson::Document* Debugger::ServeDebuggerClientRequest(const rapidjson::Document* requestDoc)
	{
		return nullptr;
	}

	rapidjson::Document* Debugger::ReadJsonFromBuffer(const uint8_t* buffer, uint32_t& bufferSize)
	{
		if (bufferSize < sizeof(uint32_t))
			return nullptr;

		uint32_t jsonSize = 0;
		::memcpy_s(&jsonSize, sizeof(jsonSize), buffer, bufferSize);
		if (jsonSize < bufferSize - sizeof(uint32_t))
			return nullptr;

		rapidjson::Document* doc = new rapidjson::Document();
		rapidjson::StringStream stream((char*)&buffer[sizeof(uint32_t)]);
		rapidjson::CursorStreamWrapper<rapidjson::StringStream> streamWrapper(stream);

		doc->ParseStream(streamWrapper);
		assert(!doc->HasParseError());
		return doc;
	}

	void Debugger::WriteJsonToBuffer(uint8_t* buffer, uint32_t& bufferSize, const rapidjson::Document* doc)
	{
		rapidjson::StringBuffer stringBuffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);

		bool accepted = doc->Accept(writer);
		assert(accepted);

		assert(stringBuffer.GetSize() <= bufferSize - sizeof(uint32_t));
		bufferSize = stringBuffer.GetSize() + sizeof(uint32_t);
		::memcpy_s(buffer, sizeof(uint32_t), &bufferSize, sizeof(uint32_t));
		::memcpy_s(&buffer[sizeof(uint32_t)], bufferSize, stringBuffer.GetString(), bufferSize);
	}
}