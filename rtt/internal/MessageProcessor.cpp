
#include "MessageProcessor.hpp"
#include "AtomicQueue.hpp"

namespace RTT
{

    using namespace std;
    using namespace detail;

    MessageProcessor::MessageProcessor(int queue_size)
        :a_queue( new AtomicQueue<DisposableInterface*>(queue_size) ),
         accept(false)
    {
    }

    MessageProcessor::~MessageProcessor()
    {
        delete a_queue;
    }

    bool MessageProcessor::initialize()
    {
        a_queue->clear();
        accept = true;
        return true;
    }

    void MessageProcessor::finalize()
    {
        accept = false;
    }

	void MessageProcessor::step()
    {
        // execute one command from the AtomicQueue.
        DisposableInterface* com(0);
        while ( !a_queue->isEmpty() ) {
            a_queue->dequeue( com );
            com->executeAndDispose();
        }
    }

    bool MessageProcessor::hasWork()
    {
        return ! a_queue->isEmpty();
    }

    bool MessageProcessor::process( DisposableInterface* c )
    {
        if (accept && c) {
            bool result = a_queue->enqueue( c );
            this->getActivity()->trigger();
            return result;
        }
        return false;
    }

}

