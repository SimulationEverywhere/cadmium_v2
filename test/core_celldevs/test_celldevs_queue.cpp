#define BOOST_TEST_MODULE CellDEVSQueueTests
#include <boost/test/unit_test.hpp>
#include <cadmium/celldevs/core/queue/queue.hpp>

using namespace cadmium::celldevs;

BOOST_AUTO_TEST_CASE(inertial) {
	auto buffer = OutputQueue<int>::newOutputQueue("inertial");

	// Check initial state
	BOOST_CHECK_EQUAL(buffer->nextState(), nullptr);
	BOOST_CHECK_EQUAL(buffer->nextTime(), std::numeric_limits<double>::infinity());

	// Pop buffer (should not have any effect)
	buffer->pop();
	BOOST_CHECK_EQUAL(buffer->nextState(), nullptr);
	BOOST_CHECK_EQUAL(buffer->nextTime(), std::numeric_limits<double>::infinity());

	// Add new elements to buffer
	int n = 10;
	for (int i = 1; i <= n; i++) {
		buffer->addToQueue(i, i * 4);
		BOOST_CHECK_EQUAL(*buffer->nextState(), i);
		BOOST_CHECK_EQUAL(buffer->nextTime(), i * 4);
	}

	// Pop buffer
	buffer->pop();
	BOOST_CHECK_EQUAL(buffer->nextState(), nullptr);
	BOOST_CHECK_EQUAL(buffer->nextTime(), std::numeric_limits<double>::infinity());
}

BOOST_AUTO_TEST_CASE(transport) {
	auto buffer = OutputQueue<int>::newOutputQueue("transport");

	// Check initial state
	BOOST_CHECK_EQUAL(buffer->nextState(), nullptr);
	BOOST_CHECK_EQUAL(buffer->nextTime(), std::numeric_limits<double>::infinity());

	// Pop buffer (should not have any effect)
	buffer->pop();
	BOOST_CHECK_EQUAL(buffer->nextState(), nullptr);
	BOOST_CHECK_EQUAL(buffer->nextTime(), std::numeric_limits<double>::infinity());


	// Add new elements to buffer
	int n = 10;
	for (int i = 1; i <= n; i++) {
		buffer->addToQueue(i, i * 4);
		BOOST_CHECK_EQUAL(*buffer->nextState(), 1);
		BOOST_CHECK_EQUAL(buffer->nextTime(), 4);
	}

	// Pop (gradually) all elements from buffer
	for (int i = 1; i <= n; i++) {
		// Pop buffer
		BOOST_CHECK_EQUAL(*buffer->nextState(), i);
		BOOST_CHECK_EQUAL(buffer->nextTime(), i * 4);
		buffer->pop();
	}
	BOOST_CHECK_EQUAL(buffer->nextState(), nullptr);
	BOOST_CHECK_EQUAL(buffer->nextTime(), std::numeric_limits<double>::infinity());

	// Add multiple new elements to buffer
	for (int i = 1; i <= n; i++) {
		for (int j = 1; j <= i; j++) {
			buffer->addToQueue(j, i * 4);
			BOOST_CHECK_EQUAL(*buffer->nextState(), 1);
			BOOST_CHECK_EQUAL(buffer->nextTime(), 4);
		}
	}

	// Pop (gradually) all elements from buffer
	for (int i = 1; i <= n; i++) {
		BOOST_CHECK_EQUAL(*buffer->nextState(), i);
		BOOST_CHECK_EQUAL(buffer->nextTime(), i * 4);
		buffer->pop();
	}
	BOOST_CHECK_EQUAL(buffer->nextState(), nullptr);
	BOOST_CHECK_EQUAL(buffer->nextTime(), std::numeric_limits<double>::infinity());

	// Add multiple new elements to buffer (in inverse time order
	for (int i = n; i >= 1; i--) {
		for (int j = 1; j <= i; j++) {
			buffer->addToQueue(j, i * 4);
			BOOST_CHECK_EQUAL(*buffer->nextState(), j);
			BOOST_CHECK_EQUAL(buffer->nextTime(), i * 4);
		}
	}

	// Pop (gradually) all elements from buffer
	for (int i = 1; i <= n; i++) {
		BOOST_CHECK_EQUAL(*buffer->nextState(), i);
		BOOST_CHECK_EQUAL(buffer->nextTime(), i * 4);
		buffer->pop();
	}
	BOOST_CHECK_EQUAL(buffer->nextState(), nullptr);
	BOOST_CHECK_EQUAL(buffer->nextTime(), std::numeric_limits<double>::infinity());
}

BOOST_AUTO_TEST_CASE(hybrid) {
	auto buffer = OutputQueue<int>::newOutputQueue("hybrid");

	// Check initial state
	BOOST_CHECK_EQUAL(buffer->nextState(), nullptr);
	BOOST_CHECK_EQUAL(buffer->nextTime(), std::numeric_limits<float>::infinity());

	// Pop buffer (should not have any effect)
	buffer->pop();
	BOOST_CHECK_EQUAL(buffer->nextState(), nullptr);
	BOOST_CHECK_EQUAL(buffer->nextTime(), std::numeric_limits<float>::infinity());

	// Add new elements to buffer
	int n = 10;
	for (int i = 1; i <= n; i++) {
		buffer->addToQueue(i, i * 4);
		BOOST_CHECK_EQUAL(*buffer->nextState(), 1);
		BOOST_CHECK_EQUAL(buffer->nextTime(), 4);
	}

	// Pop (gradually) all elements from buffer
	for (int i = 1; i <= n; i++) {
		// Pop buffer
		BOOST_CHECK_EQUAL(*buffer->nextState(), i);
		BOOST_CHECK_EQUAL(buffer->nextTime(), i * 4);
		buffer->pop();
	}
	BOOST_CHECK_EQUAL(buffer->nextState(), nullptr);
	BOOST_CHECK_EQUAL(buffer->nextTime(), std::numeric_limits<float>::infinity());

	// Add multiple new elements to buffer
	for (int i = 1; i <= n; i++) {
		for (int j = 1; j <= i; j++) {
			buffer->addToQueue(j, i * 4);
			BOOST_CHECK_EQUAL(*buffer->nextState(), 1);
			BOOST_CHECK_EQUAL(buffer->nextTime(), 4);
		}
	}

	// Pop (gradually) all elements from buffer
	for (int i = 1; i <= n; i++) {
		BOOST_CHECK_EQUAL(*buffer->nextState(), i);
		BOOST_CHECK_EQUAL(buffer->nextTime(), i * 4);
		buffer->pop();
	}
	BOOST_CHECK_EQUAL(buffer->nextState(), nullptr);
	BOOST_CHECK_EQUAL(buffer->nextTime(), std::numeric_limits<float>::infinity());

	// Add multiple new elements to buffer (in inverse time order)
	// Note: as the last element must be at the end of the queue, the queue will always have only one element
	for (int i = n; i >= 1; i--) {
		for (int j = 1; j <= i; j++) {
			buffer->addToQueue(j, i * 4);
			BOOST_CHECK_EQUAL(*buffer->nextState(), j);
			BOOST_CHECK_EQUAL(buffer->nextTime(), i * 4);
		}
	}

	// Pop single element from buffer
	BOOST_CHECK_EQUAL(*buffer->nextState(), 1);
	BOOST_CHECK_EQUAL(buffer->nextTime(), 4);
	buffer->pop();

	BOOST_CHECK_EQUAL(buffer->nextState(), nullptr);
	BOOST_CHECK_EQUAL(buffer->nextTime(), std::numeric_limits<float>::infinity());
}
