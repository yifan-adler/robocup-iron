/*
 * File: main.cpp
 * Author : ShiQiao Chen(陈世侨)
 * Affiliation: WuHan University of Technology
 */
#include "rdfw.hpp"

#include <boost/asio/error.hpp>
#include <boost/system/system_error.hpp>

int main(int argc, char **argv)
{
    auto rdfw = make_shared<_home::RDFW>();
    rdfw->Init(argc, argv);
    try
    {
        rdfw->Run();
    }
    catch (const boost::system::system_error &error)
    {
        const boost::system::error_code code = error.code();
        if (code == boost::asio::error::eof ||
            code == boost::asio::error::connection_reset)
        {
            cout << "#(RDFW): Server closed the connection after the test" << endl;
            return 0;
        }
        throw;
    }
    return 0;
}
