////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-grpc](https://github.com/semenovf/pfs-grpc) library.
//
// Changelog:
//      2019.10.20 Initial version
////////////////////////////////////////////////////////////////////////////////
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../doctest.h"
#include "pfs/grpc/async_client.hpp"
#include "pfs/grpc/async_server.hpp"
#include "async.pb.h"
#include "async.grpc.pb.h"
#include <atomic>
#include <map>
#include <thread>

static std::string const SERVER_ADDR = "localhost:1521";
static std::atomic_bool FINISH {false};

struct Module {
    std::string name;
    ::TestRpc::ModuleStatusEnum status;
};

struct Service
{
    std::map<std::string, Module> modules;
    ::TestRpc::ServiceStatusEnum status = ::TestRpc::TEST_SERVICE_STOPPED;
};

inline std::string to_string (::TestRpc::ModuleStatusEnum status)
{
    switch (status) {
        case ::TestRpc::MODULE_ACTIVATED: return "ACTIVATED";
        case ::TestRpc::MODULE_DEACTIVATED: return "DEACTIVATED";
        default: break;
    }
    return "<unknown-module-status>";
}

inline std::string to_string (::TestRpc::ServiceStatusEnum status)
{
    switch (status) {
        case ::TestRpc::TEST_SERVICE_STARTED: return "STARTED";
        case ::TestRpc::TEST_SERVICE_STOPPED: return "STOPPED";
        case ::TestRpc::TEST_SERVICE_ALREADY_STARTED: return "ALREADY_STARTED";
        case ::TestRpc::TEST_SERVICE_START_FAILED: return "START_FAILED";
        default: break;
    }
    return "<unknown-service-status>";
}

// package TestRpc_______________________________________________
// service TestService { ... ___________________________________|__________
//                                                              |         |
//                                                              v         v
class concrete_async_server : public pfs::grpc::async_server<TestRpc::TestService>
{
    using base_class = pfs::grpc::async_server<TestRpc::TestService>;
    using service_class = base_class::service_class;
    using service_type = base_class::service_type;

    Service _service;

public:
    concrete_async_server (std::string const & server_addr)
        : base_class(server_addr)
    {
        _service.modules = {
              { "module_0", { "module_0", ::TestRpc::MODULE_DEACTIVATED} }
            , { "module_1", { "module_1", ::TestRpc::MODULE_DEACTIVATED} }
            , { "module_2", { "module_2", ::TestRpc::MODULE_DEACTIVATED} }
            , { "module_3", { "module_3", ::TestRpc::MODULE_DEACTIVATED} }
        };
    }

    void register_Start ()
    {
//  rpc Start(StartServiceRequest) returns (ServiceStatus) {}
//                 |________________________
//                                         |
//                                         v
        using request_type = TestRpc::StartServiceRequest;

//  rpc Start(StartServiceRequest) returns (ServiceStatus) {}
//                                           |
//                                           v
        using response_type = TestRpc::ServiceStatus;

        using method_type = pfs::grpc::async_unary_method<
                  service_class
                , request_type
                , response_type>;

        auto f = [this] (request_type const &, response_type * response) {
            std::cout << "server: StartServiceRequest request\n";

            if (_service.status == ::TestRpc::TEST_SERVICE_STARTED)
                _service.status = ::TestRpc::TEST_SERVICE_ALREADY_STARTED;
            else
                _service.status = ::TestRpc::TEST_SERVICE_STARTED;

            response->set_status(_service.status);
        };

//  rpc Start(StartServiceRequest) returns (ServiceStatus) {}
//         |________________________________________________________
//                                                             |   |
        register_method<request_type, response_type>(//        v   v
                  & TestRpc::TestService::AsyncService::RequestStart
                , method_type::request_handler(f));
    }

    void register_Stop ()
    {
        using request_type = TestRpc::StopService;
        using response_type = TestRpc::ServiceStatus;
        using method_type = pfs::grpc::async_unary_method<
                  service_class
                , request_type
                , response_type>;

        auto f = [this] (request_type const &, response_type * response) {
            std::cout << "server: StopService request\n";
            _service.status = TestRpc::TEST_SERVICE_STOPPED;
            response->set_status(_service.status);
        };

        register_method<request_type, response_type>(
                  & TestRpc::TestService::AsyncService::RequestStop
                , method_type::request_handler(f));
    }

    void register_ListModules ()
    {
        using request_type = TestRpc::GetListModules;
        using response_type = TestRpc::ModuleStatus;
        using method_type = pfs::grpc::async_server_streaming_method<
                  service_class
                , request_type
                , response_type>;

        auto f = [this] (request_type const &, std::list<response_type> * responses) {
            std::cout << "server: ListModules request\n";

            for (auto & m: _service.modules) {
                responses->emplace_back();
                auto & response = responses->back();
                response.set_name(m.second.name);
                response.set_status(m.second.status);
            }
        };

        register_method<request_type, response_type>(
                  & TestRpc::TestService::AsyncService::RequestListModules
                , method_type::request_handler(f));
    }

    // rpc SendSegments(stream Segment) returns (Complete) {}
    void register_SendSegments ()
    {
        using request_type = TestRpc::Segment;
        using response_type = TestRpc::Complete;
        using method_type = pfs::grpc::async_client_streaming_method<
                  service_class
                , request_type
                , response_type>;

        auto f = [this] (std::list<request_type> const & requests, response_type * response) {
            std::cout << "server: SendSegments request\n";

            std::string id;
            int actual_total = 0;
            int sample_total = 0;

            for (auto & seg: requests) {

                // FIXME Empty request emplaced into list by process_request
                if (seg.id().empty())
                    continue;

                std::cout << "Segment:\n"
                        << "\tid   : " << seg.id() << "\n"
                        << "\tindex: " << seg.index() << "\n"
                        << "\ttotal: " << seg.total() << "\n";

                id = seg.id();
                sample_total = seg.total();
                actual_total++;
            }

            if (actual_total == sample_total) {
                response->set_id(id);
                response->set_complete(true);
            } else {
                response->set_id(id);
                response->set_complete(false);
            }
        };

        register_method<request_type, response_type>(
                  & TestRpc::TestService::AsyncService::RequestSendSegments
                , method_type::request_handler(f));
    }

    // rpc StartModules(stream StartModule) returns (stream ModuleStatus) {}
    void register_StartModules ()
    {
        using request_type = TestRpc::StartModule;
        using response_type = TestRpc::ModuleStatus;
        using method_type = pfs::grpc::async_bidi_streaming_method<
                  service_class
                , request_type
                , response_type>;

        auto f = [this] (std::list<request_type> const & requests
                , std::list<response_type> * responses) {
            std::cout << "server: StartModules request\n";

            for (auto const & rq: requests) {
                auto m = _service.modules.find(rq.name());

                if (m != _service.modules.end()) {
                    responses->emplace_back();
                    auto & response = responses->back();
                    response.set_name(m->second.name);
                    response.set_status(m->second.status);
                } else {
                    std::cerr << "*** WARN: module not found: " << rq.name() << "\n";
                }
            }
        };

        register_method<request_type, response_type>(
                  & TestRpc::TestService::AsyncService::RequestStartModules
                , method_type::request_handler(f));
    }


    void register_Finish ()
    {
        using request_type = TestRpc::FinishService;
        using response_type = TestRpc::FinishStatus;
        using method_type = pfs::grpc::async_unary_method<
                  service_class
                , request_type
                , response_type>;

        auto f = [this] (request_type const &, response_type * /*response*/) {
            std::cout << "server: FinishService request\n";
//             _service.status = TestRpc::TEST_SERVICE_STOPPED;
//             response->set_status(_service.status);
        };

        register_method<request_type, response_type>(
                  & TestRpc::TestService::AsyncService::RequestFinish
                , method_type::request_handler(f));
    }
};

// package TestRpc_______________________________________________
// service TestService { ... ___________________________________|__________
//                                                              |         |
//                                                              V         V
class concrete_async_client : public pfs::grpc::async_client<TestRpc::TestService>
{
    using base_class = pfs::grpc::async_client<TestRpc::TestService>;

private:

public:
    concrete_async_client (std::string const & server_addr)
        : base_class(server_addr)
    {}

////////////////////////////////////////////////////////////////////////////////
// rpc Start(StartServiceRequest) returns (ServiceStatus) {}
////////////////////////////////////////////////////////////////////////////////
    bool call_Start ()
    {
        auto f = [] (TestRpc::ServiceStatus const & response) {
            std::cout << "client: StartServiceRequest result: "
                    << "status=" << response.status() << "\n";
        };

        // Name 'PrepareAsyncCallStart' generates using concatenation of static
        // string 'PrepareAsync' and rpc name from .proto file ('CallStart').
        // This rule is valid for 'Simple RPC'
#if _MSC_VER
        return this->call<TestRpc::StartServiceRequest, TestRpc::ServiceStatus>(TestRpc::StartServiceRequest{}
            , &TestRpc::TestService::Stub::PrepareAsyncStart
            , pfs::grpc::async_unary<TestRpc::ServiceStatus>::response_handler(f));
#else
        return this->call(TestRpc::StartServiceRequest{}
                , & TestRpc::TestService::Stub::PrepareAsyncStart
                , pfs::grpc::async_unary<TestRpc::ServiceStatus>::response_handler(f));
#endif
    }

////////////////////////////////////////////////////////////////////////////////
// rpc Stop(StopService) returns (ServiceStatus) {}
////////////////////////////////////////////////////////////////////////////////
    bool call_Stop ()
    {
        auto f = [] (TestRpc::ServiceStatus const & response) {
            std::cout << "client: StopService result: "
                    << "status=" << response.status() << "\n";
        };

        // Same as for 'call_start'
#if _MSC_VER
        return this->call<TestRpc::StopService, TestRpc::ServiceStatus>(TestRpc::StopService{}
            , &TestRpc::TestService::Stub::PrepareAsyncStop
            , pfs::grpc::async_unary<TestRpc::ServiceStatus>::response_handler(f));
#else
        return this->call(TestRpc::StopService{}
                , & TestRpc::TestService::Stub::PrepareAsyncStop
                , pfs::grpc::async_unary<TestRpc::ServiceStatus>::response_handler(f));
#endif
    }

////////////////////////////////////////////////////////////////////////////////
// rpc ListModules(GetListModules) returns (stream ModuleStatus) {}
////////////////////////////////////////////////////////////////////////////////
    bool call_ListModules ()
    {
        using response_type = TestRpc::ModuleStatus;

        auto f = [] (std::list<response_type> const & responses) {
            for (auto const & response: responses) {
                std::cout << "Module:\n"
                        << "\tname  : " << response.name() << "\n"
                        << "\tstatus: " << to_string(response.status()) << "\n";
            }
        };

#if _MSC_VER
        return this->call<TestRpc::GetListModules, response_type>(TestRpc::GetListModules{}
            , &TestRpc::TestService::Stub::PrepareAsyncListModules
            , pfs::grpc::async_server_streaming<response_type>::response_handler(f));
#else
        return this->call(TestRpc::GetListModules{}
                , & TestRpc::TestService::Stub::PrepareAsyncListModules
                , pfs::grpc::async_server_streaming<response_type>::response_handler(f));
#endif
    }


////////////////////////////////////////////////////////////////////////////////
// rpc SendSegments(stream Segment) returns (Complete) {}
////////////////////////////////////////////////////////////////////////////////
    bool call_SendSegments ()
    {
        using request_type = TestRpc::Segment;
        using response_type = TestRpc::Complete;

        std::string const id = "#segment_id";
        request_type rq1;
        request_type rq2;
        request_type rq3;
        rq1.set_id(id); rq1.set_index(1); rq1.set_total(3);
        rq2.set_id(id); rq2.set_index(2); rq2.set_total(3);
        rq3.set_id(id); rq3.set_index(3); rq3.set_total(3);

        std::list<request_type> requests = {rq1, rq2, rq3};

        auto f = [] (response_type const & response) {
            std::cout << "client: SendSegment() result:\n"
                    << "\tid: " << response.id() << "\n"
                    << "\tcomplete: " << std::boolalpha << response.complete() << "\n";
        };

#if _MSC_VER
        return this->call<request_type, response_type>(std::move(requests)
            , &TestRpc::TestService::Stub::PrepareAsyncSendSegments
            , pfs::grpc::async_client_streaming<request_type, response_type>::response_handler(f));
#else
        return this->call(std::move(requests)
                , & TestRpc::TestService::Stub::PrepareAsyncSendSegments
                , pfs::grpc::async_client_streaming<request_type, response_type>::response_handler(f));
#endif
    }

    // rpc StartModules(stream StartModule) returns (stream ModuleStatus) {}
    bool call_StartModules ()
    {
        using request_type = TestRpc::StartModule;
        using response_type = TestRpc::ModuleStatus;

        auto f = [this] (std::list<response_type> const & responses) {
            for (auto const & response: responses) {
                std::cout << "Module:\n"
                        << "\tname  : " << response.name() << "\n"
                        << "\tstatus: " << to_string(response.status()) << "\n";
            }

            call_Finish();
        };

        request_type rq1;
        request_type rq2;

        rq1.set_name("module_0");
        rq2.set_name("module_1");

        std::list<request_type> requests = { rq1, rq2 };

        return this->call(std::move(requests)
                , & TestRpc::TestService::Stub::PrepareAsyncStartModules
                , pfs::grpc::async_bidi_streaming<request_type, response_type>::response_handler(f));
    }

    bool call_Finish ()
    {
        auto f = [] (TestRpc::FinishStatus const &) {
            std::cout << "Finalization...\n";
            FINISH.store(true);
        };

        return this->call<TestRpc::FinishService, TestRpc::FinishStatus>(TestRpc::FinishService{}
            , & TestRpc::TestService::Stub::PrepareAsyncFinish
            , pfs::grpc::async_unary<TestRpc::FinishStatus>::response_handler(f));
    }
};

TEST_CASE("Asynchronous RPC") {
    concrete_async_server server(SERVER_ADDR);
    std::thread server_thread { [& server] { server.run([] { return FINISH.load(); }); }};

    std::this_thread::sleep_for(std::chrono::seconds(1));

    concrete_async_client client(SERVER_ADDR);

    server.register_Start();
    server.register_Stop();
    server.register_ListModules();
    server.register_SendSegments();
    server.register_StartModules();
    server.register_Finish();

    std::thread client_reader { [& client] { client.process([] { return FINISH.load(); }); }};

    client.call_Start();
    client.call_Stop();
    client.call_Start();
    client.call_ListModules();
    client.call_SendSegments();
    client.call_StartModules();

    client_reader.join();
    server_thread.join();
}
