#include "i2c.h"
#include "bus_slave.h"

extern volatile I2CSlaveStatus  slave;

#define i2c_msg		(&mib_state.bus_msg)

void i2c_slave_receivedata()
{
	unsigned char data = i2c_receive();

	*(mib_state.bus_msg.last_data) = data;
	mib_state.bus_msg.checksum += data;
	mib_state.bus_msg.last_data += 1;

	//Check if we are at the end of the message
	if ((mib_state.bus_msg.last_data + mib_state.bus_msg.len) == mib_state.bus_msg.data_ptr)
	{
		slave.state = kI2CReceiveChecksumState;

		if (i2c_msg->flags & kCallbackBeforeChecksum)
		{
			bus_slave_callback();
			return; //Don't release clock, job of callback in this case
		}
	}

	i2c_release_clock();
}

void i2c_slave_receivechecksum()
{
	unsigned char check = i2c_receive();

	i2c_msg->checksum = ~i2c_msg->checksum + 1;

	slave.state = kI2CUserCallbackState;
	bus_slave_callback();

	if (check != i2c_msg->checksum)
		slave.last_error = kI2CInvalidChecksum;

	i2c_release_clock();
}

I2CLogicState i2c_slave_state()
{
	return slave.state;
}

void i2c_slave_setidle()
{
	slave.state = kI2CIdleState;
	slave.last_error = kI2CNoError;

	i2c_master_enable(); //let the master logic use the bus again if it needs.

	i2c_release_clock();
}

void i2c_slave_sendbyte()
{
	i2c_transmit(*(i2c_msg->data_ptr));

	i2c_msg->checksum += *(i2c_msg->data_ptr);
	i2c_msg->data_ptr += 1;

	if (i2c_msg->data_ptr == i2c_msg->last_data)
		slave.state = kI2CSendChecksumState;

	i2c_release_clock();
}

uint8 i2c_slave_lasterror()
{
	return slave.last_error;
}

void i2c_slave_interrupt()
{
	if (i2c_address_received())
	{
		i2c_receive();
		SSPOV = 0; //reset receive overflow flag

		i2c_master_disable(); //If we receive a valid address, the the master must not be running so take over the i2c data structures

		bus_slave_callback();
	}		
	else 
	{
		switch(slave.state)
		{
			case kI2CReceiveDataState:
			i2c_slave_receivedata();
			break;

			case kI2CReceiveChecksumState:
			i2c_slave_receivechecksum();
			break;

			case kI2CSendDataState:
			i2c_slave_sendbyte();
			break;

			case kI2CSendChecksumState:
			i2c_msg->checksum = (~i2c_msg->checksum) + 1;
			i2c_transmit(i2c_msg->checksum);

			slave.state = kI2CUserCallbackState;
			i2c_release_clock();
			break;

			case kI2CUserCallbackState:
			bus_slave_callback();
			break;

			default:
			//by default do not read bytes if we don't know what to do with them so that the master sees the 
			//error and does a bus reset.
			SSPOV = 0; //If the receive buffer is full we will nack, but if we don't clear the overflow we won't get any more interrupts
			i2c_release_clock();
			break;
		}
	}
}