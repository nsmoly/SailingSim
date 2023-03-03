/* 
 * Sailboat simulation game.
 *
 * Copyright (c) Nikolai Smolyanskiy, 2009-2010. All rights reserved. 
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute.
 */

#pragma once

#include <utility>
#include <map>
#include <queue>

typedef int FSMEvent;
#define NOT_EVENT   USHRT_MAX
#define NOT_STATE   USHRT_MAX
#define NOT_MACHINE USHRT_MAX

// Finite state machine dispatchable interface
class IFSM
{
public:
    virtual ~IFSM() { }

	virtual void SetId(unsigned short id) = 0;
	virtual unsigned short GetId() = 0;
   	virtual double GetLastCallTime() = 0;
    virtual void Dispatch(double time, FSMEvent event = NOT_EVENT) = 0;
};

// Finite state machine base class
template<class T>
class FSM : public IFSM
{
public:
    typedef void (T::*StateHandler)(double time, FSMEvent event);

    FSM(void) : m_lastCallTime(0), m_machineId(NOT_MACHINE), m_currentStateId(NOT_STATE), m_currentStateHandler(0)
    {
    }

    virtual ~FSM(void)
    {
    }

	virtual void SetId(unsigned short id)
	{
		m_machineId = id;
	}

	virtual unsigned short GetId()
	{
		return m_machineId;
	}

    virtual double GetLastCallTime()
    {
        return m_lastCallTime;
    }

    /*  Dispatches the event and cause the machine to change state when needed
        Parameters: 
        time - time since the program started in milliseconds
        event - event to be dispatched
    */
    virtual void Dispatch(double time, FSMEvent event = NOT_EVENT)
    {
        DWORD hash = ComputeHash(m_currentStateId, event);
        std::map<DWORD, std::pair<unsigned short, StateHandler> >::iterator it = m_stateSwitchTable.find(hash);
        if(it!=m_stateSwitchTable.end())
        {
            StateHandler handler = it->second.second;
            m_currentStateId = it->second.first;
            m_currentStateHandler = it->second.second;
            ((T*)this->*handler)(time, event);
        }
        else
        {
            ((T*)this->*m_currentStateHandler)(time, event);
        }

        m_lastCallTime = time;
    };

    virtual void SetCurrentState(unsigned short stateId)
    {
        m_currentStateId = stateId;
        std::map<unsigned short, StateHandler>::iterator itState = m_stateHandlers.find(stateId);
        _ASSERT(itState!=m_stateHandlers.end());
        m_currentStateHandler = itState->second;
    }

    virtual void RegisterState(unsigned short stateId, StateHandler stateHandler)
    {
        m_stateHandlers[stateId] = stateHandler;
    }

    virtual void AddTransition(unsigned short stateId, FSMEvent event, unsigned short newStateId)
    {
        std::map<unsigned short, StateHandler>::iterator itCurrentState = m_stateHandlers.find(stateId);
        std::map<unsigned short, StateHandler>::iterator itDestState    = m_stateHandlers.find(newStateId);
        _ASSERT(itCurrentState!=m_stateHandlers.end() && itDestState!=m_stateHandlers.end());

        DWORD hash = ComputeHash(stateId, event);
        m_stateSwitchTable[hash] = std::pair<unsigned short, StateHandler>(newStateId, itDestState->second);
    }
    
private:
    DWORD ComputeHash(unsigned short stateId, FSMEvent event)
    {
        return DWORD((stateId << 8) + event);
    }

	unsigned short m_machineId;
    unsigned short m_currentStateId;
    StateHandler m_currentStateHandler;
    double m_lastCallTime;  // in milliseconds, time since the program started
    std::map<unsigned short, StateHandler> m_stateHandlers;
	std::map<DWORD, std::pair<unsigned short, StateHandler> > m_stateSwitchTable;
};

// State Machine Manager. Used for dispatching events to managed state machines.
class FSMManager
{
public:
	FSMManager()
	{
	}

	~FSMManager()
	{
	}

	// Registers state machine with this manager
	void RegisterFSM(unsigned short id, std::tr1::shared_ptr<IFSM>& fsm)
	{
		fsm->SetId(id);
		m_stateMachines[id] = std::pair<std::tr1::shared_ptr<IFSM>, bool>(fsm, false);
	}

	// Submits event for processing by a given machine.
	void SendEvent(unsigned short machineId, FSMEvent event)
	{
		m_eventQueue.push(std::pair<int, FSMEvent>(machineId, event));
	}
	
	// Broadcasts a given event to all state machines
	void BroadcastEvent(FSMEvent event)
	{
		m_eventQueue.push(std::pair<int, FSMEvent>(-1, event));
	}

    /*  Processes all events in the queue for 1 frame (dispatches to correct machines in the order in which they were submitted
        If these events lead to states that generate more events then newly generated events are processed on the next frame processing
        Parameters: 
            time - time since the program started in milliseconds. All events in the queue are processed as they arrived at one time for this frame
    */
	void ProcessEvents(double time)
	{
		std::map<unsigned short, std::pair<std::tr1::shared_ptr<IFSM>, bool> >::iterator machine;
		
		size_t queueSize = m_eventQueue.size(); // remember the initial queue size to know how many events were at the start of the frame
		for(size_t i=0; i<queueSize; i++)
		{
			std::pair<int, FSMEvent>& event = m_eventQueue.front();
			if(event.first==-1)
			{
				for(machine=m_stateMachines.begin(); machine!=m_stateMachines.end(); ++machine)
				{
					machine->second.first->Dispatch(time, event.second);
                    machine->second.second = true;
				}
			}
			else
			{
				machine = m_stateMachines.find(event.first);
				if(machine!=m_stateMachines.end())
				{
					machine->second.first->Dispatch(time, event.second);
                    machine->second.second = true;
				}
			}

			m_eventQueue.pop();
		}

        // Call all machines that were not called with events. This cause the current state handler to be called with no event and allows
        // machines to do processing if they want while in this state
        for(machine=m_stateMachines.begin(); machine!=m_stateMachines.end(); ++machine)
		{
            if(machine->second.second==false)
            {
			    machine->second.first->Dispatch(time);
            }
            else
            {
                machine->second.second = false;
            }
		}
	}

private:
	std::queue< std::pair<int, FSMEvent> > m_eventQueue;
	std::map<unsigned short, std::pair<std::tr1::shared_ptr<IFSM>, bool> > m_stateMachines;
};
