#include <Algin.h>
#include <EntryPoint.h>

class Sandbox : public AG::Application
{
public:
	Sandbox() {

	}

	~Sandbox() {

	}
};

AG::Application* AG::CreateApplication() {
	return new Sandbox();
}