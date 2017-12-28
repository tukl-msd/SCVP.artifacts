#include <iostream>
#include <systemc.h>
#include <tlm.h>

using namespace std;

class exampleInitiator: sc_module, tlm::tlm_bw_transport_if<>
{
    public:
    tlm::tlm_initiator_socket<> iSocket;
    SC_CTOR(exampleInitiator) : iSocket("iSocket")
    {
        iSocket.bind(*this);
        SC_THREAD(process);
    }

    // Dummy method:
    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                   sc_dt::uint64 end_range)
    {
        SC_REPORT_FATAL(this->name(),"invalidate_direct_mem_ptr not implement");
    }

    // Dummy method:
    tlm::tlm_sync_enum nb_transport_bw(
            tlm::tlm_generic_payload& trans,
            tlm::tlm_phase& phase,
            sc_time& delay)
    {
        SC_REPORT_FATAL(this->name(),"nb_transport_bw is not implemented");
        return tlm::TLM_ACCEPTED;
    }

    private:
    void process()
    {
        // Write to memory1:
        for (int i = 0; i < 4; i++) {
            tlm::tlm_generic_payload trans;
            unsigned char data = rand();
            trans.set_address(i);
            trans.set_data_length(1);
            trans.set_streaming_width(1);
            trans.set_command(tlm::TLM_WRITE_COMMAND);
            trans.set_data_ptr(&data);
            trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
            sc_time delay = sc_time(0, SC_NS);
            iSocket->b_transport(trans, delay);
            if ( trans.is_response_error() )
              SC_REPORT_FATAL(name(), "Response error from b_transport");
            wait(delay);
            std::cout << "@" << sc_time_stamp() << " Write Data: "
                      << (unsigned int)data << std::endl;
        }
        // Write to memory1:
        for (int i = 512; i < 515; i++) {
            tlm::tlm_generic_payload trans;
            unsigned char data = rand();
            trans.set_address(i);
            trans.set_data_length(1);
            trans.set_streaming_width(1);
            trans.set_command(tlm::TLM_WRITE_COMMAND);
            trans.set_data_ptr(&data);
            trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
            sc_time delay = sc_time(0, SC_NS);
            iSocket->b_transport(trans, delay);
            if ( trans.is_response_error() )
              SC_REPORT_FATAL(name(), "Response error from b_transport");
            wait(delay);
            std::cout << "@" << sc_time_stamp() << " Write Data: "
                      << (unsigned int)data << std::endl;

        }

        // Read from memory1:
        for (int i = 0; i < 4; i++) {
            tlm::tlm_generic_payload trans;
            unsigned char data;
            trans.set_address(i);
            trans.set_data_length(1);
            trans.set_streaming_width(1);
            trans.set_command(tlm::TLM_READ_COMMAND);
            trans.set_data_ptr(&data);
            trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
            sc_time delay = sc_time(0, SC_NS);
            iSocket->b_transport(trans, delay);
            if ( trans.is_response_error() )
              SC_REPORT_FATAL(name(), "Response error from b_transport");
            wait(delay);
            std::cout << "@" << sc_time_stamp() << " Read Data: "
                      << (unsigned int)data << std::endl;
        }
        // Read from memory2:
        for (int i = 512; i < 515; i++) {
            tlm::tlm_generic_payload trans;
            unsigned char data;
            trans.set_address(i);
            trans.set_data_length(1);
            trans.set_streaming_width(1);
            trans.set_command(tlm::TLM_READ_COMMAND);
            trans.set_data_ptr(&data);
            trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
            sc_time delay = sc_time(0, SC_NS);
            iSocket->b_transport(trans, delay);
            if ( trans.is_response_error() )
              SC_REPORT_FATAL(name(), "Response error from b_transport");
            wait(delay);
            std::cout << "@" << sc_time_stamp() << " Read Data: "
                      << (unsigned int)data << std::endl;
        }
    }
};

class exampleTarget : sc_module, tlm::tlm_fw_transport_if<>
{
    private:
    unsigned char mem[512];

    public:
    tlm::tlm_target_socket<> tSocket;

    SC_CTOR(exampleTarget) : tSocket("tSocket")
    {
        tSocket.bind(*this);
    }

    void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay)
    {
        if (trans.get_address() >= 512)
        {
             trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
             return;
        }

        if (trans.get_data_length() != 1)
        {
             trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
             return;
        }

        if(trans.get_command() == tlm::TLM_WRITE_COMMAND)
        {
            memcpy(&mem[trans.get_address()], // destination
                   trans.get_data_ptr(),      // source
                   trans.get_data_length());  // size
        }
        else // (trans.get_command() == tlm::TLM_READ_COMMAND)
        {
            memcpy(trans.get_data_ptr(),      // destination
                   &mem[trans.get_address()], // source
                   trans.get_data_length());  // size
        }

        delay = delay + sc_time(40, SC_NS);

        trans.set_response_status( tlm::TLM_OK_RESPONSE );
    }

    // Dummy method
    virtual tlm::tlm_sync_enum nb_transport_fw(
            tlm::tlm_generic_payload& trans,
            tlm::tlm_phase& phase,
            sc_time& delay )
    {
        SC_REPORT_FATAL(this->name(),"nb_transport_fw is not implemented");
        return tlm::TLM_ACCEPTED;
    }

    // Dummy method
    bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans,
                            tlm::tlm_dmi& dmi_data)
    {
        SC_REPORT_FATAL(this->name(),"get_direct_mem_ptr is not implemented");
        return false;
    }

    // Dummy method
    unsigned int transport_dbg(tlm::tlm_generic_payload& trans)
    {
        SC_REPORT_FATAL(this->name(),"transport_dbg is not implemented");
        return 0;
    }

};

class exampleInterconnect : sc_module,
                            tlm::tlm_bw_transport_if<>,
                            tlm::tlm_fw_transport_if<>
{
    public:
    tlm::tlm_initiator_socket<> iSocket[2];
    tlm::tlm_target_socket<> tSocket;

    SC_CTOR(exampleInterconnect)
    {
        tSocket.bind(*this);
        iSocket[0].bind(*this);
        iSocket[1].bind(*this);
    }

    void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay)
    {
        // Annotate Bus Delay
        delay = delay + sc_time(40, SC_NS);

        if(trans.get_address() < 512)
        {
            iSocket[0]->b_transport(trans, delay);
        }
        else
        {
            // Correct Address:
            trans.set_address(trans.get_address() - 512);
            iSocket[1]->b_transport(trans, delay);

            // Correct Address:
            trans.set_address(trans.get_address() + 512);
        }
    }

    // Dummy method
    virtual tlm::tlm_sync_enum nb_transport_fw(
            tlm::tlm_generic_payload& trans,
            tlm::tlm_phase& phase,
            sc_time& delay )
    {
        SC_REPORT_FATAL(this->name(),"nb_transport_fw is not implemented");
        return tlm::TLM_ACCEPTED;
    }

    // Dummy method
    bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans,
                            tlm::tlm_dmi& dmi_data)
    {
        SC_REPORT_FATAL(this->name(),"get_direct_mem_ptr is not implemented");
        return false;
    }

    // Dummy method
    unsigned int transport_dbg(tlm::tlm_generic_payload& trans)
    {
        SC_REPORT_FATAL(this->name(),"transport_dbg is not implemented");
        return 0;
    }

    // Dummy method:
    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                   sc_dt::uint64 end_range)
    {
        SC_REPORT_FATAL(this->name(),"invalidate_direct_mem_ptr not implement");
    }

    // Dummy method:
    tlm::tlm_sync_enum nb_transport_bw(
            tlm::tlm_generic_payload& trans,
            tlm::tlm_phase& phase,
            sc_time& delay)
    {
        SC_REPORT_FATAL(this->name(),"nb_transport_bw is not implemented");
        return tlm::TLM_ACCEPTED;
    }

};

int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{

    exampleInitiator * cpu    = new exampleInitiator("cpu");
    exampleTarget * memory1   = new exampleTarget("memory1");
    exampleTarget * memory2   = new exampleTarget("memory2");
    exampleInterconnect * bus = new exampleInterconnect("bus");

    cpu->iSocket.bind(bus->tSocket);
    bus->iSocket[0].bind(memory1->tSocket);
    bus->iSocket[1].bind(memory2->tSocket);

    sc_start();

    return 0;
}