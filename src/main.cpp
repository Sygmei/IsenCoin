#include <iostream>

#include <Transaction.hpp>

int main(int argc, char** argv)
{
    using namespace ic;

    Chain chain;
    chain.create_wallet();

    std::cout << "Hello world!" << std::endl;
    Transaction(0, 1, 5);
    return 0;
}