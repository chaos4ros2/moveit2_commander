#include <memory>
#include <boost/python.hpp>
#include <boost/thread/mutex.hpp>
#include <ros/init.h>

#include "conversions.h"

namespace moveit {
namespace python {

class InitProxy {
public:
	static void init(const std::string& node_name="moveit_python_wrapper",
	                 const boost::python::dict& remappings = boost::python::dict());
	static void shutdown();

	~InitProxy();

private:
	InitProxy(const std::string& node_name, const boost::python::dict& remappings);

	static boost::mutex lock;
	static std::unique_ptr<InitProxy> singleton_instance;

private:
	std::unique_ptr<ros::AsyncSpinner> spinner;
};
boost::mutex InitProxy::lock;
std::unique_ptr<InitProxy> InitProxy::singleton_instance;

void InitProxy::init(const std::string& node_name, const boost::python::dict& remappings)
{
	boost::mutex::scoped_lock slock(lock);
	if (!singleton_instance && !ros::isInitialized())
		singleton_instance.reset(new InitProxy(node_name, remappings));
}

void InitProxy::shutdown()
{
	boost::mutex::scoped_lock slock(lock);
	singleton_instance.reset();
}

InitProxy::InitProxy(const std::string& node_name, const boost::python::dict& remappings)
{
	ros::init(fromDict<std::string>(remappings), node_name, ros::init_options::AnonymousName | ros::init_options::NoSigintHandler);
	spinner.reset(new ros::AsyncSpinner(1));
	spinner->start();
}

InitProxy::~InitProxy()
{
	spinner->stop();
	spinner.reset();
}

BOOST_PYTHON_FUNCTION_OVERLOADS(roscpp_init_overloads, InitProxy::init, 0, 2)

void export_ros_init()
{
	boost::python::def("roscpp_init", InitProxy::init, roscpp_init_overloads());
	boost::python::def("roscpp_shutdown", &InitProxy::shutdown);
}

} }