#ifndef STATEMACHINE_STATE_HPP
#define STATEMACHINE_STATE_HPP

#include <statemachine_fwd.hpp>
#include <stringref.hpp>

#include <cassert>
#include <iterator>

namespace statemachine
{
namespace detail
{

//! \brief A state in a state machine.
//!
//! The State describes a state in a finite state machine.
template <typename TOptions>
class State
{
public:
    using state_machine_type = StateMachine<TOptions>;
    using transition_type = Transition<TOptions>;

    //! \brief The possible child modes.
    //!
    //! This enum lists the possible child modes.
    //! - Exclusive: The children are active exclusively (i.e. one and only
    //!   one child will be active).
    //! - Parallel: The children are active parallely (i.e. all children
    //!   will be active simultanously).
    enum ChildMode
    {
        Exclusive,
        Parallel
    };

    template <bool TIsConst>
    class TransitionIterator;

    //! The iterator type for iterating over transitions.
    typedef TransitionIterator<false> transition_iterator;
    //! The const-iterator type for iterating over transitions.
    typedef TransitionIterator<true> const_transition_iterator;


    //! \brief Constructs a state.
    //!
    //! Constructs a state with given \p name which will be a child of the
    //! \p parent state. The \p parent may be a null-pointer. In this case
    //! the state is at the root of its hierarchy.
    explicit State(const char* name, State* parent = 0);

    State(const State&) = delete;
    State& operator=(const State&) = delete;

    //! \brief Returns the child mode.
    //!
    //! Returns the current child mode. The default is Exclusive.
    ChildMode childMode() const
    {
        return m_childMode;
    }



    transition_iterator beginTransitions()
    {
        return transition_iterator(m_transitions);
    }

    const_transition_iterator beginTransitions() const
    {
        return const_transition_iterator(m_transitions);
    }

    const_transition_iterator cbeginTransitions() const
    {
        return const_transition_iterator(m_transitions);
    }

    transition_iterator endTransitions()
    {
        return transition_iterator();
    }

    const_transition_iterator endTransitions() const
    {
        return const_transition_iterator();
    }

    const_transition_iterator cendTransitions() const
    {
        return const_transition_iterator();
    }


    State* findChild(StringRef name) const;

    //! \brief Checks if the state is active.
    //!
    //! Returns \p true, if the state is active, which means that it belongs
    //! to the current state machine configuration.
    bool isActive() const;

    //! \brief Checks for atomicity.
    //!
    //! Returns \p true, if this state is atomic which means that it does not
    //! have children.
    bool isAtomic() const
    {
        return m_children == 0;
    }

    //! \brief Checks for a compound state.
    //!
    //! Returns \p true, if this state is a compound state, i.e. it does
    //! have at least one child and the children are active exclusively.
    //! One and only one child of an active compound state will be active.
    bool isCompound() const
    {
        return !isAtomic() && m_childMode == Exclusive;
    }

    //! \brief Checks for a parallel state.
    //!
    //! Returns \p true, if this state is a parallel state, i.e. it does
    //! have at least one child and the children are active in parallel.
    //! All children of an active parallel state will be active.
    bool isParallel() const
    {
        return !isAtomic() && m_childMode == Parallel;
    }

    //! \brief The name.
    //!
    //! Returns the state's name.
    const char* name() const
    {
        return m_name;
    }

    //! \brief The on-entry method.
    //!
    //! This method is called by the state machine, whenever this state
    //! is entered. The event which triggered the configuration change
    //! is passed in \p event.
    virtual void onEntry(unsigned /*event*/)
    {
        // The default implementation does nothing.
    }

    //! \brief The on-exit method.
    //!
    //! This method is called by the state machine, when the staet is left.
    //! The event which triggered the configuration change is passed in
    //! \p event.
    virtual void onExit(unsigned /*event*/)
    {
        // The default implementation does nothing.
    }

    virtual void enterInvoke()
    {
        // The default implementation does nothing.
    }

    virtual void exitInvoke()
    {
        // The default implementation does nothing.
    }

    //! \brief The parent.
    //!
    //! Returns the state's parent or a null-pointer, if the state has no
    //! parent.
    State* parent() const
    {
        return m_parent;
    }

    //! \brief Sets the child mode.
    //!
    //! Sets the child mode to \p mode. Setting the child mode to Exclusive,
    //! will turn the state into a compound state. When the child mode is
    //! set to Parallel, this state will become a parallel state.
    //!
    //! \note The state machine's configuration won't be updated, if the child
    //! mode of one of its states changes. Changing the child mode while
    //! the associated state machine is running, results in undefined
    //! behaviour.
    void setChildMode(ChildMode mode)
    {
        m_childMode = mode;
    }

    //! \brief Changes the parent.
    //!
    //! Removes this child from its old parent and makes it a child of the
    //! new \p parent state. If \p parent is a null-pointer, this state will
    //! be at the root of its hieararchy.
    //!
    //! \note Changing the parent state while the associated state machine
    //! is running results in undefined behaviour.
    void setParent(State* parent);

    //! \brief Returns the state machine.
    //!
    //! Returns the state machine to which this state belongs. If the state
    //! has not been added to a state machine, yet, a null-pointer is
    //! returned.
    state_machine_type* stateMachine() const
    {
        return m_stateMachine;
    }

    // -------------------------------------------------------------------------
    // Iterators
    // -------------------------------------------------------------------------

    //! \brief An iterator over a state's transitions.
    template <bool TIsConst>
    class TransitionIterator
    {
    public:
        typedef std::ptrdiff_t difference_type;
        typedef transition_type value_type;
        typedef typename std::conditional<TIsConst, const transition_type*,
                                          transition_type*>::type pointer;
        typedef typename std::conditional<TIsConst, const transition_type&,
                                          transition_type&>::type reference;
        typedef std::forward_iterator_tag iterator_category;

        //! Default constructs an end-iterator.
        TransitionIterator()
            : m_current(0)
        {
        }

        //! A copy-constructor with implicit conversion from a non-const
        //! iterator.
        TransitionIterator(const TransitionIterator<false>& other)
            : m_current(other.m_current)
        {
        }

        //! Prefix increment.
        TransitionIterator& operator++()
        {
            m_current = m_current->m_nextInSourceState;
            return *this;
        }

        //! Postfix increment.
        TransitionIterator operator++ (int)
        {
            TransitionIterator temp(m_current);
            m_current = m_current->m_next;
            return temp;
        }

        //! Returns \p true, if this iterator is equal to the \p other iterator.
        bool operator== (TransitionIterator other) const
        {
            return m_current == other.m_current;
        }

        //! Returns \p true, if this iterator is not equal to the \p other
        //! iterator.
        bool operator!= (TransitionIterator other) const
        {
            return m_current != other.m_current;
        }

        //! Returns a reference to the transition.
        reference operator* () const
        {
            return *m_current;
        }

        //! Returns a pointer to the transition.
        pointer operator-> () const
        {
            return m_current;
        }

    private:
        //! The current transition.
        pointer m_current;

        //! Constructs an iterator pointing to the \p transition.
        TransitionIterator(pointer transition)
            : m_current(transition)
        {
        }

        friend class State;
        // Befriend the non-const version with the const version.
        friend TransitionIterator<true>;
    };

private:
    enum Flags
    {
        SkipTransitionSelection = 0x01,
        InEnterSet              = 0x02,
        InExitSet               = 0x04
    };

    //! The state's name.
    const char* m_name;
    //! The parent state.
    State* m_parent;
    //! The associated state machine.
    state_machine_type* m_stateMachine;
    //! A pointer to the first child.
    State* m_children;
    //! A pointer to the next sibling in the linked list.
    State* m_nextSibling;
    //! A pointer to the first transition.
    transition_type* m_transitions;
    //! This flag specifies if the children are active exclusively or parallely.
    ChildMode m_childMode;
    int m_flags; //! \todo This should be of type Flags

    bool m_internalActive;
    bool m_visibleActive;


    //! Adds a \p child.
    void addChild(State* child);
    //! Removes a \p child.
    void removeChild(State* child);

    //! Adds a \p transition.
    void pushBackTransition(transition_type* transition) noexcept;

    friend state_machine_type;

    template <typename TDerived>
    friend class EventDispatcherBase;
};

template <typename TOptions>
State<TOptions>::State(const char* name, State* parent)
    : m_name(name),
      m_parent(parent),
      m_stateMachine(parent ? parent->m_stateMachine : 0),
      m_children(0),
      m_nextSibling(0),
      m_transitions(0),
      m_childMode(Exclusive),
      m_internalActive(false),
      m_visibleActive(false)
{
    if (parent)
        parent->addChild(this);
}

template <typename TOptions>
State<TOptions>* State<TOptions>::findChild(StringRef name) const
{
#if 0
    const State* parent = this;
    while (1)
    {
        std::pair<StringRef, StringRef> splitName = name.split('.');
        if (splitName.first.empty())
            return 0;

        const_child_iterator child = parent->child_begin();
        for (; child != parent->child_end(); ++child)
        {
            if (child->name() == splitName.first)
                break;
        }
        if (child == parent->child_end())
            return 0;
        if (splitName.second.empty())
            return const_cast<State*>(&*child);

        parent = &*child;
        name = splitName.second;
    }
#endif
    return 0;
}

template <typename TOptions>
bool State<TOptions>::isActive() const
{
    if (m_stateMachine)
    {
        // TODO:
        //m_stateMachine->m_configurationChangeMutex.lock();
        bool active = m_visibleActive;
        //m_stateMachine->m_configurationChangeMutex.unlock();
        return active;
    }
    else
    {
        assert(!m_visibleActive);
        return false;
    }
}

template <typename TOptions>
void State<TOptions>::setParent(State* parent)
{
    if (parent == m_parent)
        return;

    if (m_parent)
        m_parent->removeChild(this);
    if (parent)
        parent->addChild(this);

    // Propagate the state machine of the new parent to the new child states.
    state_machine_type* newFsm = parent ? parent->stateMachine() : 0;
    state_machine_type* fsm = newFsm
                              ? newFsm
                              : (m_parent ? m_parent->stateMachine() : 0);
    if (fsm)
    {
        for (auto iter = fsm->subtree_begin(this),
              end_iter = fsm->subtree_end(this);
             iter != end_iter; ++iter)
        {
            iter->m_stateMachine = newFsm;
        }
    }

    m_parent = parent;
}

// ----=====================================================================----
//     Private methods
// ----=====================================================================----

template <typename TOptions>
void State<TOptions>::addChild(State* child)
{
    assert(child->m_nextSibling == 0);

    if (!m_children)
    {
        m_children = child;
    }
    else
    {
        State* iter = m_children;
        while (iter->m_nextSibling)
            iter = iter->m_nextSibling;
        iter->m_nextSibling = child;
    }
}

template <typename TOptions>
void State<TOptions>::removeChild(State* child)
{
    assert(m_children != 0);

    if (child == m_children)
        m_children = child->m_nextSibling;
    else
    {
        State* iter = m_children;
        while (1)
        {
            assert(iter != 0);
            if (iter->m_nextSibling == child)
            {
                iter->m_nextSibling = child->m_nextSibling;
                break;
            }
            iter = iter->m_nextSibling;
        }
    }
    child->m_nextSibling = 0;
}

template <typename TOptions>
void State<TOptions>::pushBackTransition(transition_type* transition) noexcept
{
    if (!m_transitions)
    {
        m_transitions = transition;
    }
    else
    {
        transition_type* iter = m_transitions;
        while (iter->m_nextInSourceState)
            iter = iter->m_nextInSourceState;
        iter->m_nextInSourceState = transition;
    }
}

#if 0
const State* findLeastCommonAncestor(const State* state1, const State* state2);

//! \brief Checks if a state is an ancestor of another state.
//!
//! Returns \p true, if \p ancestor is an ancestor of \p descendant. Every
//! state is its own ancestor.
bool isAncestor(const State* ancestor, const State* descendant);

inline
bool isDescendant(const State* descendant, const State* ancestor)
{
    return isAncestor(ancestor, descendant);
}

inline
bool isProperAncestor(const State* ancestor, const State* descendant)
{
    return isAncestor(ancestor, descendant->parent());
}
#endif

} // namespace detail
} // namespace statemachine

#endif // STATEMACHINE_STATE_HPP
