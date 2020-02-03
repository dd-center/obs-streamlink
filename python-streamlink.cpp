#include "python-streamlink.h"
#include <obs-module.h>

#include <windows.h>
namespace streamlink {
    bool loaded = false;
    bool loadingFailed = false;

    PyObject* module;
    namespace methods
    {
        PyObject* new_session;
    }

    using ThreadState = PyGILState_STATE;

    ThreadState AcquireThreadState()
    {
        return PyGILState_Ensure();
    }

    void ReleaseThreadState(ThreadState state) {
        PyGILState_Release(state);
    }

    std::string PyStringToString(PyObject* pyStr)
    {
        ssize_t len;
        auto cstr = PyUnicode_AsUTF8AndSize(pyStr, &len);
        auto str = std::string(cstr, len);

        return str;
    }
    std::string GetExceptionInfo()
    {
        if (!PyErr_Occurred()) return "";
        PyObject* type, * value, * traceback;
        PyErr_Fetch(&type, &value, &traceback);
        auto strObj = PyObject_ASCII(value);

        std::string message = PyStringToString(strObj);
        Py_DECREF(strObj);

        PyErr_Clear();
        return message;
    }
    void LogFailure()
    {
        std::cerr << GetExceptionInfo() << std::endl;
    }

    void Initialize()
    {
        auto FireInitializationFailure = [](bool log = true) -> void
        {
            loaded = false;
            loadingFailed = true;
            if (log)
                LogFailure();
            PyEval_ReleaseThread(PyThreadState_Get());
        };
        if (!Py_IsInitialized())
        {
            // TODO make this configurable via properties.
            std::string data_path = obs_get_module_data_path(obs_current_module());
            std::string python_path = data_path.append("/Python38");
            std::wstring widstr = std::wstring(python_path.begin(), python_path.end());
            Py_SetPythonHome(widstr.c_str());
            Py_Initialize();
        }

        module = PyImport_ImportModule("streamlink");
        if (module == nullptr) return FireInitializationFailure();

        methods::new_session = PyObject_GetAttrString(module, static_cast<const char*>("Streamlink"));
        if (methods::new_session == nullptr) return FireInitializationFailure();
        if (!PyCallable_Check(methods::new_session)) return FireInitializationFailure(false);

        loaded = true;
        PyEval_ReleaseThread(PyThreadState_Get());
    }
    
    PyObjectHolder::PyObjectHolder(PyObject* underlying, bool inc) : underlying(underlying)
    {
        if (inc)
            Py_INCREF(underlying);
    }
    PyObjectHolder::~PyObjectHolder()
    {
        if (underlying != nullptr)
            Py_DECREF(underlying);
    }
    PyObjectHolder::PyObjectHolder(PyObjectHolder&& another) noexcept
    {
        underlying = another.underlying;
        another.underlying = nullptr;
    }
    PyObjectHolder& PyObjectHolder::operator=(PyObjectHolder&& another) noexcept
    {
        underlying = another.underlying;
        another.underlying = nullptr;

        return *this;
    }
    
    Stream::Stream(PyObject* underlying) : PyObjectHolder(underlying)
    {
    }
    Stream::Stream(Stream&& another) noexcept : PyObjectHolder(std::move(another))
    {
    }
    ssize_t Stream::Read(unsigned char* buf, const ssize_t len)
    {
        auto iucallable = PyObject_GetAttrString(underlying, "read");
        if (iucallable == nullptr)
            throw invalid_underlying_object();
        auto iucallableHolder = PyObjectHolder(iucallable, false);
        if (!PyCallable_Check(iucallable)) throw invalid_underlying_object();

        auto args = PyTuple_Pack(1, PyLong_FromLong(len));
        auto argsGuard = PyObjectHolder(args, false);

        auto result = PyObject_Call(iucallable, args, nullptr);
        if (result == nullptr)
            throw call_failure(GetExceptionInfo().c_str());
        auto resultGuard = PyObjectHolder(result, false);

        char* buf1;
        ssize_t readLen;
        PyBytes_AsStringAndSize(result, &buf1, &readLen);
        std::memcpy(buf, buf1, readLen);

        if (readLen == 0) throw stream_ended();
        return readLen;
    }
    void Stream::Close()
    {
        auto args = PyTuple_New(0);
        auto argsGuard = PyObjectHolder(args, false);

        auto closeCallable = PyObject_GetAttrString(underlying, "close");
        if (closeCallable == nullptr)
            throw invalid_underlying_object();
        auto closeCallableHolder = PyObjectHolder(closeCallable, false);
        if (!PyCallable_Check(closeCallable)) throw invalid_underlying_object();
        auto result = PyObject_Call(closeCallable, args, nullptr);
        if (result == nullptr)
            throw call_failure(GetExceptionInfo().c_str());
    }
    StreamInfo::StreamInfo(std::string name, PyObject* underlying) : PyObjectHolder(underlying), name(std::move(name))
    {

    }
    StreamInfo::StreamInfo(StreamInfo&& another) noexcept : PyObjectHolder(std::move(another))
    {
        name = another.name;
    }
    PyObject* StreamInfo::Open()
    {
        auto callable = PyObject_GetAttrString(underlying, "open");
        if (!callable) throw invalid_underlying_object();
        auto callableGuard = PyObjectHolder(callable, false);
        if (!PyCallable_Check(callable)) throw invalid_underlying_object();

        auto args = PyTuple_New(0);
        auto argsGuard = PyObjectHolder(args, false);
        auto result = PyObject_Call(callable, args, nullptr);

        if (result == nullptr)
            throw call_failure(GetExceptionInfo().c_str());
        return result;
    }
    
    ThreadGIL::ThreadGIL()
    {
        state = PyGILState_Ensure();
    }

    ThreadGIL::~ThreadGIL()
    {
        PyGILState_Release(state);
    }
}

streamlink::Session::Session()
{
    //while (!IsDebuggerPresent())
        //Sleep(100);
    //DebugBreak();
    if (!loaded) throw not_loaded();
    auto args = PyTuple_New(0);
    underlying = PyObject_Call(methods::new_session, args, nullptr);
    Py_DECREF(args);

    if (underlying == nullptr)
        throw call_failure(GetExceptionInfo().c_str());

    set_option = PyObject_GetAttrString(underlying, static_cast<const char*>("set_option"));
    if (set_option == nullptr) throw call_failure(GetExceptionInfo().c_str());
    set_optionGuard = PyObjectHolder(set_option, false);
    if (!PyCallable_Check(set_option)) throw invalid_underlying_object();
}

namespace streamlink {
    std::map<std::string, StreamInfo> Session::GetStreamsFromUrl(std::string url)
    {
        auto streamsCallable = PyObject_GetAttrString(underlying, static_cast<const char*>("streams"));
        if (streamsCallable == nullptr) throw call_failure(GetExceptionInfo().c_str());
        auto streamsCallableGuard = PyObjectHolder(streamsCallable, false);
        if (!PyCallable_Check(streamsCallable)) throw invalid_underlying_object();

        if (!loaded) throw not_loaded();
        auto urlStrObj = PyUnicode_FromStringAndSize(url.c_str(), url.size());
        auto urlStrObjGuard = PyObjectHolder(urlStrObj, false);
        auto args = PyTuple_Pack(1, urlStrObj);
        auto argsGuard = PyObjectHolder(args, false);

        auto result = PyObject_Call(streamsCallable, args, nullptr);
        if (result == nullptr)
            throw call_failure(GetExceptionInfo().c_str());

        auto resultGuard = PyObjectHolder(result, false);
        auto items = PyDict_Items(result);
        auto itemsGuard = PyObjectHolder(items, false);

        auto size = PyList_Size(items);
        auto ret = std::map<std::string, StreamInfo>();
        for (int i = 0; i < size; i++)
        {
            auto itemTuple = PyList_GetItem(items, i);
            if (PyTuple_Size(itemTuple) != 2) continue;
            auto nameObj = PyTuple_GetItem(itemTuple, 0);
            auto valueObj = PyTuple_GetItem(itemTuple, 1);
            auto name = PyStringToString(nameObj);

            ret.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(name, valueObj));
            //ret.emplace(name, StreamInfo(name, valueObj));
        }
        return ret;
    }
}

void streamlink::Session::SetOption(std::string const& name, PyObject* value)
{
    if (!loaded) throw not_loaded();
    auto nameObj = PyUnicode_FromStringAndSize(name.c_str(), name.size());
    auto nameObjGuard = PyObjectHolder(nameObj, false);

    auto args = PyTuple_Pack(2, nameObj, value);
    auto argsGuard = PyObjectHolder(args, false);

    auto result = PyObject_Call(set_option, args, nullptr);
    if (result == nullptr)
        throw call_failure(GetExceptionInfo().c_str());
    Py_DECREF(result);
}

void streamlink::Session::SetOptionString(std::string const& name, std::string const& value)
{
    auto valueObj = PyUnicode_FromStringAndSize(value.c_str(), value.size());
    auto valueObjGuard = PyObjectHolder(valueObj, false);
    SetOption(name, valueObj);
}

void streamlink::Session::SetOptionDouble(std::string const& name, double value)
{
    auto valueObj = PyFloat_FromDouble(value);
    auto valueObjGuard = PyObjectHolder(valueObj, false);
    SetOption(name, valueObj);
}

void streamlink::Session::SetOptionInt(std::string const& name, long long value)
{
    auto valueObj = PyLong_FromLongLong(value);
    auto valueObjGuard = PyObjectHolder(valueObj, false);
    SetOption(name, valueObj);
}

void streamlink::Session::SetOptionBool(std::string const& name, bool value)
{
    auto valueObj = PyBool_FromLong(value);
    auto valueObjGuard = PyObjectHolder(valueObj, false);
    SetOption(name, valueObj);
}
