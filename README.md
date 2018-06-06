# QtService
A platform independent library to easily create system services and use some of their features

[![Travis Build Status](https://travis-ci.org/Skycoder42/QtService.svg?branch=master)](https://travis-ci.org/Skycoder42/QtService)
[![Appveyor Build status](https://ci.appveyor.com/api/projects/status/6mag0932qeloxuru?svg=true)](https://ci.appveyor.com/project/Skycoder42/qtservice)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/8596bb56c0df40c0bba7ddd28da65fee)](https://www.codacy.com/app/Skycoder42/QtService)
[![AUR](https://img.shields.io/aur/version/qt5-service.svg)](https://aur.archlinux.org/packages/qt5-service/)

## Features
- Allows you to create system (and user) services for various systems
	- Provides a single Interface to implement only once without having to care to much about the platform
	- Actual platform specifc stuff is provided via plugins, making it easy to support other backends as well
	- Most backends support native logging (i.e. journald, syslog, winevents, ...)
	- Supports socket-activation (for systemd & launchd)
	- Interfaces can be used to handle general service events
- Easy testing/debugging by simply switching the service backend via the start arguments
- Provides very basic service control to, e.g. get the status of a service or start/stop it
- Provides generic callbacks to allow realtively easy access to platform specific features
- Comes with a "Terminal frontend mode" which allows to create a CLI interface that communicates with the actual service without any extra coding needed
	- Terminals will connect to the running service. If none is running, they can start it (on some platforms)
	- Terminals send their command line arguments to the running service
	- A connected terminal can send input to the master (via stdin) and receive output (stdout)
	- Terminals can disconnect by beeing closed, leaving the service running (even the one that started the master)
	- The Service gets a "Terminal" handle for each connected terminal, allowing parallel connections. Each terminal implements QIODevice, making communication easy
	- For all this no extra code is needed from your side
- Supported service backends:
	- Systemd (For most modern Linux Distros)
	- Windows Services
	- Launchd (Tested on MacOs, but should theoretically work on other unixes as well)
	- Android Services
	- "Standard Services" - A platform independed dummy backend that runs services as a normal proccess - useful for testing, debugging or as fallback.
- QtCreator template to easily create a service project, with configuration file templates for each platform

**Note:** What this framework does explicitly *not* handle is "service installation" - as that is extremly different on all platforms and cannot be easily generalized. Performing the service installation, registration or whatever the platform needs is up to the developer. However, all the files that are needed as metadata for a service on each platform are provided via the QtCreator project template, which makes it significantly easier to deploy your service, as all thats left is to correctly "apply" those.

Details on what exactly is supported by each backend can be found in the doxygen documentation under the "Related Pages" tab.

## Download/Installation
1. Package Managers: The library is available via:
	- **Arch-Linux:** AUR-Repository: [`qt5-service`](https://aur.archlinux.org/packages/qt5-service/)
	- **Ubuntu:** Launchpad-PPA: [ppa:skycoder42/qt-modules](https://launchpad.net/~skycoder42/+archive/ubuntu/qt-modules), package `libqt5service[1/-dev]`
	- **MacOs:**
		- Tap: [`brew tap Skycoder42/qt-modules`](https://github.com/Skycoder42/homebrew-qt-modules)
		- Package: `qtservice`
		- **IMPORTANT:** Due to limitations of homebrew, you must run `source /usr/local/opt/qtservice/bashrc.sh` before you can use the module.
2. Simply add my repository to your Qt MaintenanceTool (Image-based How-To here: [Add custom repository](https://github.com/Skycoder42/QtModules/blob/master/README.md#add-my-repositories-to-qt-maintenancetool)):
	1. Open the MaintenanceTool, located in your Qt install directory (e.g. `~/Qt/MaintenanceTool`)
	2. Select `Add or remove components` and click on the `Settings` button
	3. Go to `Repositories`, scroll to the bottom, select `User defined repositories` and press `Add`
	4. In the right column (selected by default), type:
		- On Linux: https://install.skycoder42.de/qtmodules/linux_x64
		- On Windows: https://install.skycoder42.de/qtmodules/windows_x86
		- On Mac: https://install.skycoder42.de/qtmodules/mac_x64
	5. Press `Ok`, make shure `Add or remove components` is still selected, and continue the install (`Next >`)
	6. A new entry appears under all supported Qt Versions (e.g. `Qt > Qt 5.11 > Skycoder42 Qt modules`)
	7. You can install either all of my modules, or select the one you need: `Qt Service`
	8. Continue the setup and thats it! you can now use the module for all of your installed Kits for that Qt Version
3. Download the compiled modules from the release page. **Note:** You will have to add the correct ones yourself and may need to adjust some paths to fit your installation! In addition to that, you will have to download the modules this one depends on as well.
4. Build it yourself! **Note:** This requires all build an runtime dependencies to be available. If you don't have/need cmake, you can ignore the related warnings. To automatically build and install to your Qt installation, run:
	- `qmake`
	- `make qmake_all`
	- `make` (If you want the tests/examples/etc. run `make all`)
	- Optional steps:
		- `make doxygen` to generate the documentation
		- `make lrelease` to generate the translations
	- `make install`

## Usage
The C++ part of the Service is relatively simple. All you have to do is implement the `QtService::Service` interface and start it in your main. The most important methods of the service are `start` and `stop` - which are called when the service was started or stopped...

```.cpp
class TestService : public QtService::Service
{
	Q_OBJECT

public:
	explicit TestService(int &argc, char **argv) : //the reference here is important!
		Service{argc, argv}
	{}

protected:
	CommandMode onStart() override {
		qDebug() << "Service was started";
		return Synchronous; //service is now assumed started
	}

	CommandMode onStop(int &exitCode) override {
		qDebug() << "Stop received";
		// do some complicated stopping stuff asynchronously that takes some time...
		QTimer::singleShot(3000, this, [this](){
			emit stopped(EXIT_SUCCESS);
		});
		return Asynchronous; // service is still stopping until "stopped" has been emitted
	}
};

int main(int argc, char *argv[]) {
	//IMPORTANT: do NOT create a QCoreApplication here - this is done internally by the backends!
	TestService service{argc, argv};
	return service.exec();
}
```

Actually running the service depends on your choosen backend. When testing with the standard backend, you can simply run the service without any parameters, or the backend explicitly specified:

```
/path/to/service --backend standard
```

To run with a system backend, you need to register the executable somehow in the service system of your operating system. The trick here is to register it with the backend as parameter. So for example, on windows you would register the service as `\path\to\service --backend windows`. For more details check the documentation or create a service project from the template - it will create service files that already do this for you.

### Terminals
To allow terminals, you must set the `QtService::Service::terminalActive` property to `true` and implement the `QtService::Service::terminalConnected` and optionally the `QtService::Service::verifyCommand` methods. The service is started the same way as usual. Internally, you will get the terminals as soon as they have connected and can then process their arguments and communicate with them.

To start a terminal client, simply prepend `--terminal` as well as the backend the service is running on to the command line. It is recommended to create a small wrapper script for each platform that takes care of these parameters. On linux, for example, you would create a shell script like below that simply starts the executable with the additional parameters:

```.sh
#!/bin/sh
# use as "service-cli <arguments>" (assuming this script is named service-cli)
exec /path/to/service -platform minimal --backend systemd --terminal "$@"
```

(The platform argument is recommended to not depend on any windowing system)

### Service Control
The `QtService::ServiceControl` allows you to control services by sending commands to them and retrieving the status. However, what exactly is possible greatly varies for each platform. Always use `QtService::ServiceControl::supportFlags` to figure out what you can actually do on the current platform. You can also check the doxygen documentation to get an overview over all the backends and their features.

## Documentation
The documentation is available on [github pages](https://skycoder42.github.io/QtService/). It was created using [doxygen](http://www.doxygen.org/). The HTML-documentation and Qt-Help files are shipped together with the module for both the custom repository and the package on the release page. Please note that doxygen docs do not perfectly integrate with QtCreator/QtAssistant.

## References
The Service backend code for the windows plugin was created by using the code from https://github.com/qtproject/qt-solutions as a basis.
