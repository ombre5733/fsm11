#ifndef STATEMACHINE_STATEMACHINE_HPP
#define STATEMACHINE_STATEMACHINE_HPP

#include "eventdispatcher.hpp"
#include "state.hpp"
#include "statemachine_fwd.hpp"
#include "transition.hpp"

//#include "/home/manuel/code/weos/src/scopeguard.hpp"


#include <deque>
#include <mutex>
#include <iterator>
#include <type_traits>


// DEBUG
#include <iostream>

namespace statemachine
{
namespace detail
{

#if 0
//! \brief A state machine visitor.
//!
//! The Visitor is the base class for all visitors which operate on a
//! state machine. It walks the state hierarchy in a depth-first manner
//! and can be used for implementing both pre-order and post-order traversal.
//!
//! For a pre-order traversal, the derived class has to implement one or
//! more of the <tt>visit*State()</tt> methods. For every state \p s, the
//! following sequence is executed:
//! \code
//! visitState(s);
//! if (s->isAtomic()) visitAtomicState(s);
//! if (!s->isAtomic()) visitNonAtomicState(s);
//! if (s->isCompound()) visitCompoundState(s);
//! if (s->isParallel()) visitParallelState(s);
//! \endcode
//!
//! The <tt>leave*State()</tt> methods are needed for a post-order traversal.
//! Their calling sequence is exactly the reverse of the corresponding
//! pre-order methods and so the following code is executed for every state
//! \p s:
//! \code
//! if (s->isParallel()) leaveParallelState(s);
//! if (s->isCompound()) leaveCompoundState(s);
//! if (!s->isAtomic()) leaveNonAtomicState(s);
//! if (s->isAtomic()) leaveAtomicState(s);
//! leaveState(s);
//! \endcode
template <typename TDerived>
class Visitor
{
public:
    //! \brief Applies the visitor to a state machine.
    //!
    //! Applies this visitor to the state machine \p machine.
    void operator() (StateMachine& machine);

    //! \brief Called from the state machine.
    //!
    //! This method is called from the state machine for every visited
    //! \p state. It dispatches the kind of the \p state and calls into
    //! the user-defined visit*State() methods.
    //! \internal
    void doVisit(state_type* state)
    {
        derived().visitState(state);
        if (state->isAtomic())
        {
            derived().visitAtomicState(state);
        }
        else
        {
            derived().visitNonAtomicState(state);
            if (state->isCompound())
                derived().visitCompoundState(state);
            else
                derived().visitParallelState(state);
        }
    }

    //! \brief Called from the state machine.
    //!
    //! This method is called from the state machine for every \p state
    //! to be left. It dispatches the kind of the \p state and calls into
    //! the user-defined leave*State() methods.
    //! \internal
    void doLeave(state_type* state)
    {
        if (state->isAtomic())
        {
            derived().leaveAtomicState(state);
        }
        else
        {
            if (state->isCompound())
                derived().leaveCompoundState(state);
            else
                derived().leaveParallelState(state);
            derived().leaveNonAtomicState(state);
        }
        derived().leaveState(state);
    }

    //! \brief Visits a state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given \p state.
    void visitState(state_type* /*state_type*/) {}

    //! \brief Visits an atomic state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given atomic \p state.
    void visitAtomicState(state_type* /*state_type*/) {}

    //! \brief Visits a non-state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given non-atomic \p state.
    void visitNonAtomicState(state_type* /*state_type*/) {}

    //! \brief Visits a compound state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given compound \p state.
    void visitCompoundState(state_type* /*state_type*/) {}

    //! \brief Visits a parallel state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given parallel \p state.
    void visitParallelState(state_type* /*state_type*/) {}


    //! \brief Leaves a state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given \p state.
    void leaveState(state_type* /*state_type*/) {}

    //! \brief Leaves an atomic state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given atomic \p state.
    void leaveAtomicState(state_type* /*state_type*/) {}

    //! \brief Leaves a non-atomic state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given non-atomic \p state.
    void leaveNonAtomicState(state_type* /*state_type*/) {}

    //! \brief Leaves a compound state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given compound \p state.
    void leaveCompoundState(state_type* /*state_type*/) {}

    //! \brief Leaves a parallel state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given parallel \p state.
    void leaveParallelState(state_type* /*state_type*/) {}

private:
    //! Casts this instance to the derived class.
    TDerived& derived()
    {
        return *static_cast<TDerived*>(this);
    }
};

//! \brief A state machine configuration visitor.
//!
//! The ConfigurationVisitor is the base class for all visitors which operate
//! a state machine's configuration, i.e. on the set of active states of a
//! state machine. It walks the state hierarchy in a depth-first manner
//! and can be used for implementing both pre-order and post-order traversal.
//!
//! All the properties of a Visitor apply to the ConfigurationVisitor, too,
//! except that the callbacks are only invoked when the state is active.
template <typename TDerived>
class ConfigurationVisitor
{
public:
    //! \brief Applies the visitor to a state machine.
    //!
    //! Applies this visitor to the state machine \p machine.
    void operator() (StateMachine& machine);

    //! \brief Called from the state machine.
    //!
    //! This method is called from the state machine for every visited
    //! \p state. It dispatches the kind of the \p state and calls into
    //! the user-defined visit*State() methods.
    //! \internal
    void doVisit(state_type* state)
    {
        //assert(state->isActive());

        derived().visitState(state);
        if (state->isAtomic())
        {
            derived().visitAtomicState(state);
        }
        else
        {
            derived().visitNonAtomicState(state);
            if (state->isCompound())
                derived().visitCompoundState(state);
            else
                derived().visitParallelState(state);
        }
    }

    //! \brief Called from the state machine.
    //!
    //! This method is called from the state machine for every \p state
    //! to be left. It dispatches the kind of the \p state and calls into
    //! the user-defined leave*State() methods.
    //! \internal
    void doLeave(state_type* state)
    {
        //assert(state->isActive());

        if (state->isAtomic())
        {
            derived().leaveAtomicState(state);
        }
        else
        {
            if (state->isCompound())
                derived().leaveCompoundState(state);
            else
                derived().leaveParallelState(state);
            derived().leaveNonAtomicState(state);
        }
        derived().leaveState(state);
    }

    //! \brief Visits a state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given \p state.
    void visitState(state_type* /*state_type*/) {}

    //! \brief Visits an atomic state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given atomic \p state.
    void visitAtomicState(state_type* /*state_type*/) {}

    //! \brief Visits a non-state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given non-atomic \p state.
    void visitNonAtomicState(state_type* /*state_type*/) {}

    //! \brief Visits a compound state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given compound \p state.
    void visitCompoundState(state_type* /*state_type*/) {}

    //! \brief Visits a parallel state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given parallel \p state.
    void visitParallelState(state_type* /*state_type*/) {}


    //! \brief Leaves a state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given \p state.
    void leaveState(state_type* /*state_type*/) {}

    //! \brief Leaves an atomic state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given atomic \p state.
    void leaveAtomicState(state_type* /*state_type*/) {}

    //! \brief Leaves a non-atomic state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given non-atomic \p state.
    void leaveNonAtomicState(state_type* /*state_type*/) {}

    //! \brief Leaves a compound state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given compound \p state.
    void leaveCompoundState(state_type* /*state_type*/) {}

    //! \brief Leaves a parallel state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given parallel \p state.
    void leaveParallelState(state_type* /*state_type*/) {}

private:
    //! Casts this instance to the derived class.
    TDerived& derived()
    {
        return *static_cast<TDerived*>(this);
    }
};
#endif



template <typename... TTypes>
class Storage
{
    typedef std::tuple<TTypes...> tuple_type;

public:
    template <int TIndex>
    typename std::tuple_element<TIndex, tuple_type>::type load()
    {
        return std::get<TIndex>(m_data);
    }

    template <int TIndex, typename TType>
    void store(TType&& element)
    {
        std::get<TIndex>(m_data) = element;
    }

private:
    tuple_type m_data;
};

template <>
class Storage<>
{
public:
    template <int TIndex>
    void load()
    {
        static_assert(TIndex != TIndex, "No storage specified");
    }

    template <int TIndex, typename TType>
    void store(TType&& element)
    {
        static_assert(TIndex != TIndex, "No storage specified");
    }
};






//! \brief A finite state machine.
//!
//! The StateMachine implements the logic for a finite state machine.
//!
//! Every state machine has an implicit root state. All top-level states of
//! a state machine, must be added as child of this root state.
template <typename TOptions>
class StateMachine : public get_dispatcher<TOptions>::type,
                     public Storage<>
{
public:
    using type = StateMachine<TOptions>;
    using state_type = State<TOptions>;
    using transition_type = Transition<TOptions>;
    using event_type = typename TOptions::event_type;
    using event_list_type = typename TOptions::event_list_type;

private:
    using dispatcher_type = typename get_dispatcher<TOptions>::type;
    friend dispatcher_type;

public:
    StateMachine()
        : m_rootState("(StateMachine)"),
          m_enabledTransitions(0)
    {
        m_rootState.m_stateMachine = this;
    }

    //! \brief Destroys the state machine.
    virtual ~StateMachine()
    {
        this->stop();
    }

    StateMachine(const StateMachine&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;

//    template <typename TDerived>
//    void apply(Visitor<TDerived>& visitor);

//    template <typename TDerived>
//    void apply(Visitor<TDerived>&& visitor)
//    {
//        apply(visitor);
//    }

//    template <typename TDerived>
//    void apply(ConfigurationVisitor<TDerived>& visitor);

    //! \brief Adds a transition.
    //!
    //! Adds a transition, which will be created from a transition specification
    //! \p spec.
    template <typename TState, typename TEvent, typename TGuard,
              typename TAction>
    void add(SourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>&& t);

    //! \brief Adds a transition.
    //!
    //! Adds a transition, which will be created from a transition specification
    //! \p spec. The method returns this state machine.
    //StateMachine& operator<< (const detail::TransitionSpec& spec);

    //! \note Calling this method directly is almost certainly a
    //! mistake. Instead an RAII-lock such as std::lock_guard<> or
    //! std::unique_lock<> should be used as in the following example:
    //! \code
    //! StateMachine sm;
    //! std::unique_lock<StateMachine> lock(sm);
    //! ... operate on the state machine ...
    //! \endcode
    void lock();

    //! \note Calling this method directly is almost certainly a mistake.
    //!
    //! \sa lock()
    void unlock();

    void waitForConfigurationChange(std::unique_lock<StateMachine>& lock);

    //! \brief The root state.
    //!
    //! Returns the root state. The root state is implicitly created by the
    //! state machine. Every top-level state must be a child of the
    //! root state.
    state_type* rootState() const
    {
        return const_cast<state_type*>(&m_rootState);
    }

    operator state_type*()
    {
        return &m_rootState;
    }

    operator const state_type*() const
    {
        return &m_rootState;
    }

    // -------------------------------------------------------------------------
    // Iterators
    // -------------------------------------------------------------------------

    template <bool TIsConst>
    class PreOrderIterator;
    typedef PreOrderIterator<false> pre_order_iterator;
    typedef PreOrderIterator<true>  const_pre_order_iterator;
    typedef pre_order_iterator iterator;
    typedef const_pre_order_iterator const_iterator;

    template <bool TIsConst>
    class PostOrderIterator;
    typedef PostOrderIterator<false> post_order_iterator;
    typedef PostOrderIterator<true>  const_post_order_iterator;

    template <bool TIsConst>
    class AtomicIterator;
    typedef AtomicIterator<false> atomic_iterator;
    typedef AtomicIterator<true> const_atomic_iterator;

    template <bool TIsConst>
    class SubTreeIterator;
    typedef SubTreeIterator<false> subtree_iterator;
    typedef SubTreeIterator<true> const_subtree_iterator;

    template <bool TIsConst>
    class SiblingIterator;
    typedef SiblingIterator<false> sibling_iterator;
    typedef SiblingIterator<true> const_sibling_iterator;


    //! \brief A pre-order iterator to the first state.
    //!
    //! Returns a pre-order iterator to the first state.
    pre_order_iterator begin()
    {
        return pre_order_begin();
    }

    //! \brief A pre-order const-iterator to the first state.
    //!
    //! Returns a pre-order const-iterator to the first state.
    const_pre_order_iterator begin() const
    {
        return pre_order_begin();
    }

    //! \brief A pre-order const-iterator to the first state.
    //!
    //! Returns a pre-order const-iterator to the first state.
    const_pre_order_iterator cbegin() const
    {
        return pre_order_cbegin();
    }

    //! \brief A pre-order iterator past the last state.
    //!
    //! Returns a pre-order iterator past the last state.
    pre_order_iterator end()
    {
        return pre_order_end();
    }

    //! \brief A pre-order const-iterator past the last state.
    //!
    //! Returns a pre-order const-iterator past the last state.
    const_pre_order_iterator end() const
    {
        return pre_order_end();
    }

    //! \brief A pre-order const-iterator past the last state.
    //!
    //! Returns a pre-order const-iterator past the last state.
    const_pre_order_iterator cend() const
    {
        return pre_order_cend();
    }


    //! \brief A pre-order iterator to the first state.
    //!
    //! Returns a pre-order iterator to the first state.
    pre_order_iterator pre_order_begin()
    {
        return pre_order_iterator(&m_rootState);
    }

    //! \brief A pre-order const-iterator to the first state.
    //!
    //! Returns a pre-order const-iterator to the first state.
    const_pre_order_iterator pre_order_begin() const
    {
        return const_pre_order_iterator(&m_rootState);
    }

    //! \brief A pre-order const-iterator to the first state.
    //!
    //! Returns a pre-order const-iterator to the first state.
    const_pre_order_iterator pre_order_cbegin() const
    {
        return const_pre_order_iterator(&m_rootState);
    }

    //! \brief A pre-order iterator past the last state.
    //!
    //! Returns a pre-order iterator past the last state.
    pre_order_iterator pre_order_end()
    {
        return pre_order_iterator();
    }

    //! \brief A pre-order const-iterator past the last state.
    //!
    //! Returns a pre-order const-iterator past the last state.
    const_pre_order_iterator pre_order_end() const
    {
        return const_pre_order_iterator();
    }

    //! \brief A pre-order const-iterator past the last state.
    //!
    //! Returns a pre-order const-iterator past the last state.
    const_pre_order_iterator pre_order_cend() const
    {
        return const_pre_order_iterator();
    }


    //! \brief A post-order iterator to the first state.
    //!
    //! Returns a post-order iterator to the first state.
    post_order_iterator post_order_begin()
    {
        state_type* state = &m_rootState;
        while (state->m_children)
            state = state->m_children;
        return post_order_iterator(state);
    }

    //! \brief A post-order const-iterator to the first state.
    //!
    //! Returns a post-order const-iterator to the first state.
    const_post_order_iterator post_order_begin() const
    {
        return post_order_cbegin();
    }

    //! \brief A post-order const-iterator to the first state.
    //!
    //! Returns a post-order const-iterator to the first state.
    const_post_order_iterator post_order_cbegin() const
    {
        return const_cast<StateMachine*>(this)->post_order_begin();
    }

    //! \brief A post-order iterator past the last state.
    //!
    //! Returns a post-order iterator past the last state.
    post_order_iterator post_order_end()
    {
        return post_order_iterator();
    }

    //! \brief A post-order const-iterator past the last state.
    //!
    //! Returns a post-order const-iterator past the last state.
    const_post_order_iterator post_order_end() const
    {
        return const_post_order_iterator();
    }

    //! \brief A post-order const-iterator past the last state.
    //!
    //! Returns a post-order const-iterator past the last state.
    const_post_order_iterator post_order_cend() const
    {
        return const_post_order_iterator();
    }


    //! \brief An iterator to the first atomic state.
    //!
    //! Returns an iterator to the first atomic state.
    atomic_iterator atomic_begin()
    {
        state_type* state = &m_rootState;
        while (state->m_children)
            state = state->m_children;
        return atomic_iterator(state);
    }

    //! \brief A const-iterator to the first atomic state.
    //!
    //! Returns a const-iterator to the first atomic state.
    const_atomic_iterator atomic_begin() const
    {
        return atomic_cbegin();
    }

    //! \brief A const-iterator to the first atomic state.
    //!
    //! Returns a const-iterator to the first atomic state.
    const_atomic_iterator atomic_cbegin() const
    {
        return const_cast<StateMachine*>(this)->atomic_begin();
    }

    //! \brief An iterator past the last atomic state.
    //!
    //! Returns an iterator past the last atomic state.
    atomic_iterator atomic_end()
    {
        return atomic_iterator();
    }

    //! \brief A const-iterator past the last atomic state.
    //!
    //! Returns a const-iterator past the last atomic state.
    const_atomic_iterator atomic_end() const
    {
        return const_atomic_iterator();
    }

    //! \brief A const-iterator past the last atomic state.
    //!
    //! Returns a const-iterator past the last atomic state.
    const_atomic_iterator atomic_cend() const
    {
        return const_atomic_iterator();
    }


    pre_order_iterator subtree_begin(state_type* root)
    {
        // TODO: make this a template
        // template <typename TIterator>
        // TIterator subtree_begin(state_type* root);
        return pre_order_iterator(root);
    }

    const_pre_order_iterator subtree_begin(const state_type* root) const
    {
        return const_pre_order_iterator(root);
    }

    const_pre_order_iterator subtree_cbegin(const state_type* root) const
    {
        return const_pre_order_iterator(root);
    }

    pre_order_iterator subtree_end(state_type* root)
    {
        pre_order_iterator iter(root);
        if (root)
        {
            iter.skipChildren();
            ++iter;
        }
        return iter;
    }

    const_pre_order_iterator subtree_end(const state_type* root) const
    {
        return subtree_cend(root);
    }

    const_pre_order_iterator subtree_cend(const state_type* root) const
    {
        return const_cast<StateMachine*>(this)->subtree_end(
                    const_cast<state_type*>(root));
    }

    //! \brief A pre-order iterator.
    //!
    //! The PreOrderIterator is a pre-order depth-first iterator over the
    //! states of a state machine. If \p TIsConst is set, it implements
    //! a const-iterator, otherwise the accessed state is mutable.
    template <bool TIsConst>
    class PreOrderIterator
    {
    public:
        typedef std::ptrdiff_t difference_type;
        typedef state_type value_type;
        typedef typename std::conditional<TIsConst, const state_type*, state_type*>::type
                    pointer;
        typedef typename std::conditional<TIsConst, const state_type&, state_type&>::type
                    reference;
        typedef std::forward_iterator_tag iterator_category;


        //! Default constructs an end-iterator.
        PreOrderIterator() noexcept
            : m_current(0),
              m_skipChildren(false)
        {
        }

        //! (Copy-)Constructs an iterator from a non-const iterator.
        PreOrderIterator(const PreOrderIterator<false>& other) noexcept
            : m_current(other.m_current),
              m_skipChildren(false)
        {
        }

        //! Prefix increment.
        PreOrderIterator& operator++() noexcept
        {
            //assert(m_current)
            if (m_current->m_children && !m_skipChildren)
            {
                // Visit the first child.
                m_current = m_current->m_children;
            }
            else
            {
                m_skipChildren = false;
                // This state has no child, so go to its sibling. If there is
                // no sibling, go to the parent's sibling, the parent's
                // parent's sibling... Note that the direct ancestors must
                // be skipped because they have been visited already.
                while (m_current->m_nextSibling == 0)
                {
                    m_current = m_current->parent();
                    if (!m_current)
                        return *this;
                }
                m_current = m_current->m_nextSibling;
            }
            return *this;
        }

        //! Postfix increment.
        PreOrderIterator operator++(int) noexcept
        {
            PreOrderIterator temp(*this);
            ++*this;
            return temp;
        }

        //! Returns \p true, if this iterator is equal to the \p other iterator.
        bool operator==(PreOrderIterator other) const noexcept
        {
            return m_current == other.m_current;
        }

        //! Returns \p true, if this iterator is not equal to the \p other
        //! iterator.
        bool operator!=(PreOrderIterator other) const noexcept
        {
            return m_current != other.m_current;
        }

        //! Returns a reference to the state.
        reference operator*() const noexcept
        {
            return *m_current;
        }

        //! Returns a pointer to the state.
        pointer operator->() const noexcept
        {
            return m_current;
        }

        //! \brief Skip child iteration.
        //!
        //! When this method is called, the iterator will skip the children
        //! when it is advanced the next time. The flag is reset automatically.
        void skipChildren() noexcept
        {
            m_skipChildren = true;
        }


        //! \brief Returns an iterator to the first child.
        //!
        //! Returns an iterator to the first child.
        sibling_iterator child_begin() noexcept
        {
            return sibling_iterator(m_current->m_children);
        }

        //! \brief Returns a const-iterator to the first child.
        //!
        //! Returns a const-iterator to the first child.
        const_sibling_iterator child_begin() const noexcept
        {
            return const_sibling_iterator(m_current->m_children);
        }

        //! \brief Returns a const-iterator to the first child.
        //!
        //! Returns a const-iterator to the first child.
        const_sibling_iterator child_cbegin() const noexcept
        {
            return const_sibling_iterator(m_current->m_children);
        }

        //! \brief Returns an iterator past the last child.
        //!
        //! Returns an iterator past the last child.
        sibling_iterator child_end() noexcept
        {
            return sibling_iterator();
        }

        //! \brief Returns a const-iterator past the last child.
        //!
        //! Returns a const-iterator past the last child.
        const_sibling_iterator child_end() const noexcept
        {
            return const_sibling_iterator();
        }

        //! \brief Returns a const-iterator past the last child.
        //!
        //! Returns a const-iterator past the last child.
        const_sibling_iterator child_cend() const noexcept
        {
            return const_sibling_iterator();
        }

    private:
        //! The current state.
        pointer m_current;
        //! The flag is set, when the children of a state have to be skipped.
        bool m_skipChildren;

        //! Constructs an iterator pointing to the \p state.
        PreOrderIterator(pointer state) noexcept
            : m_current(state),
              m_skipChildren(false)
        {
        }


        friend class StateMachine;
        // Befriend the non-const version with the const version.
        friend PreOrderIterator<true>;
    };

    //! \brief A post-order iterator.
    //!
    //! The PostOrderIterator is a post-order depth-first iterator over the
    //! states of a state machine. If \p TIsConst is set, it implements
    //! a const-iterator, otherwise the accessed state is mutable.
    template <bool TIsConst>
    class PostOrderIterator
    {
    public:
        typedef std::ptrdiff_t difference_type;
        typedef state_type value_type;
        typedef typename std::conditional<TIsConst, const state_type*, state_type*>::type
                    pointer;
        typedef typename std::conditional<TIsConst, const state_type&, state_type&>::type
                    reference;
        typedef std::forward_iterator_tag iterator_category;


        //! Default constructs an end-iterator.
        PostOrderIterator() noexcept
            : m_current(0)
        {
        }

        //! A copy-constructor with implicit conversion from a non-const
        //! iterator.
        PostOrderIterator(const PostOrderIterator<false>& other) noexcept
            : m_current(other.m_current)
        {
        }

        //! Prefix increment.
        PostOrderIterator& operator++() noexcept
        {
            //assert(m_current)

            if (m_current->m_nextSibling)
            {
                m_current = m_current->m_nextSibling;
                while (m_current->m_children)
                    m_current = m_current->m_children;
            }
            else
            {
                m_current = m_current->parent();
            }
            return *this;
        }

        //! Postfix increment.
        PostOrderIterator operator++(int) noexcept
        {
            PostOrderIterator temp(*this);
            ++*this;
            return temp;
        }

        //! Returns \p true, if this iterator is equal to the \p other iterator.
        bool operator==(PostOrderIterator other) const noexcept
        {
            return m_current == other.m_current;
        }

        //! Returns \p true, if this iterator is not equal to the \p other
        //! iterator.
        bool operator!=(PostOrderIterator other) const noexcept
        {
            return m_current != other.m_current;
        }

        //! Returns a reference to the state.
        reference operator*() const noexcept
        {
            return *m_current;
        }

        //! Returns a pointer to the state.
        pointer operator->() const noexcept
        {
            return m_current;
        }

    private:
        //! The current state.
        pointer m_current;

        //! Constructs an iterator pointing to the \p state.
        PostOrderIterator(pointer state) noexcept
            : m_current(state)
        {
        }


        friend class StateMachine;
        // Befriend the non-const version with the const version.
        friend PostOrderIterator<true>;
    };

    //! \brief An iterator over atomic states.
    //!
    //! The AtomicIterator is an iterator over the atomic states of a
    //! state machine. If \p TIsConst is set, it implements a const-iterator,
    //! otherwise the accessed state is mutable.
    template <bool TIsConst>
    class AtomicIterator
    {
    public:
        typedef std::ptrdiff_t difference_type;
        typedef state_type value_type;
        typedef typename std::conditional<TIsConst, const state_type*, state_type*>::type
                    pointer;
        typedef typename std::conditional<TIsConst, const state_type&, state_type&>::type
                    reference;
        typedef std::forward_iterator_tag iterator_category;


        //! Default constructs an end-iterator.
        AtomicIterator() noexcept
            : m_current(0)
        {
        }

        //! A copy-constructor with implicit conversion from a non-const
        //! iterator.
        AtomicIterator(const AtomicIterator<false>& other) noexcept
            : m_current(other.m_current)
        {
        }

        //! Prefix increment.
        AtomicIterator& operator++() noexcept
        {
            //assert(m_current)

            // Per definitionem, this iterator points to an atomic state.
            // The state's direct ancestors are uninteresting as they cannot
            // be atomic. So the iterator can only advance to the next sibling
            // or to the parent's sibling, or the parent's parent's sibling...
            while (m_current->m_nextSibling == 0)
            {
                m_current = m_current->parent();
                if (!m_current)
                    return *this;
            }
            m_current = m_current->m_nextSibling;
            while (m_current->m_children)
                m_current = m_current->m_children;

            return *this;
        }

        //! Postfix increment.
        AtomicIterator operator++(int) noexcept
        {
            AtomicIterator temp(*this);
            ++*this;
            return temp;
        }

        //! Returns \p true, if this iterator is equal to the \p other iterator.
        bool operator==(AtomicIterator other) const noexcept
        {
            return m_current == other.m_current;
        }

        //! Returns \p true, if this iterator is not equal to the \p other
        //! iterator.
        bool operator!=(AtomicIterator other) const noexcept
        {
            return m_current != other.m_current;
        }

        //! Returns a reference to the state.
        reference operator*() const noexcept
        {
            return *m_current;
        }

        //! Returns a pointer to the state.
        pointer operator->() const noexcept
        {
            return m_current;
        }

    private:
        //! The current state.
        pointer m_current;

        //! Constructs an iterator pointing to the \p state.
        AtomicIterator(pointer state) noexcept
            : m_current(state)
        {
        }


        friend class StateMachine;
        // Befriend the non-const version with the const version.
        friend AtomicIterator<true>;
    };

    //! \brief An iterator over state siblings.
    template <bool TIsConst>
    class SiblingIterator
    {
    public:
        typedef std::ptrdiff_t difference_type;
        typedef state_type value_type;
        typedef typename std::conditional<TIsConst, const state_type*, state_type*>::type
                    pointer;
        typedef typename std::conditional<TIsConst, const state_type&, state_type&>::type
                    reference;
        typedef std::forward_iterator_tag iterator_category;


        //! Default constructs an end-iterator.
        SiblingIterator() noexcept
            : m_current(0)
        {
        }

        //! A copy-constructor with implicit conversion from a non-const
        //! iterator.
        SiblingIterator(const SiblingIterator<false>& other) noexcept
            : m_current(other.m_current)
        {
        }

        //! Prefix increment.
        SiblingIterator& operator++() noexcept
        {
            m_current = m_current->m_nextSibling;
            return *this;
        }

        //! Postfix increment.
        SiblingIterator operator++(int) noexcept
        {
            SiblingIterator temp(*this);
            ++*this;
            return temp;
        }

        //! Returns \p true, if this iterator is equal to the \p other iterator.
        bool operator==(SiblingIterator other) const noexcept
        {
            return m_current == other.m_current;
        }

        //! Returns \p true, if this iterator is not equal to the \p other
        //! iterator.
        bool operator!=(SiblingIterator other) const noexcept
        {
            return m_current != other.m_current;
        }

        //! Returns a reference to the state.
        reference operator*() const noexcept
        {
            return *m_current;
        }

        //! Returns a pointer to the state.
        pointer operator->() const noexcept
        {
            return m_current;
        }

    private:
        //! The current child state.
        pointer m_current;

        //! Constructs an iterator pointing to the \p state.
        SiblingIterator(pointer state) noexcept
            : m_current(state)
        {
        }


        friend class StateMachine;
        // Befriend the non-const version with the const version.
        friend SiblingIterator<true>;
    };

private:
    //! A list of events which have to be handled by the event loop.
    event_list_type m_eventList;

    //! The implicit root state.
    state_type m_rootState;

    //! A mutex to prevent concurrent modifications.
    mutable std::mutex m_mutex;

    std::condition_variable m_configurationChangeCondition;


    //! The set of enabled transitions.
    transition_type* m_enabledTransitions;



    //! Broadcasts a configuration change.
    void broadcastConfigurationChange();


    // TODO: Move all these functions into EventDispatcherBase
    //! Clears the flags of all states.
    void clearStateFlags()
    {
        for (pre_order_iterator iter = begin(); iter != end(); ++iter)
            iter->m_flags = 0;
    }

    //! Clears the set of enabled transitions.
    void clearEnabledTransitionsSet();

    //! Enters all states in the enter-set.
    void enterStatesInEnterSet(unsigned event);

    //! Leaves the current configuration, which effectively stops the
    //! state machine.
    void leaveConfiguration();

    //! Leaves all states in the exit-set.
    void leaveStatesInExitSet(unsigned event);

    //! Performs a microstep.
    void microstep(event_type event);

    //! \brief Propagates the entry mark to all descendant states.
    //!
    //! Loops over all states in the state machine. If a state \p S is marked
    //! for entry, one of its children (in case \p S is a compound state) or
    //! all of its children (in case \p S is a parallel state) will be marked
    //! for entry, too.
    void markDescendantsForEntry();

    //! \brief Selects matching transitions.
    //!
    //! Loops over all states and selects all transitions matching the given
    //! criteria. If \p eventless is set, only transitions without events are
    //! selected. Otherwise, a transition is selected, if it's trigger event
    //! equals the given \p event.
    void selectTransitions(bool eventless, event_type event);

    //! Computes the transition domain of the given \p transition.
    static state_type* transitionDomain(const transition_type* transition)
    {
        //! \todo Make this a free function
        return transition->source(); //! HACK!!!!!!!!   THIS IS WRONG!!!!!
    }

    friend class EventDispatcherBase<StateMachine>;
};

template <typename TOptions>
template <typename TState, typename TEvent, typename TGuard,
          typename TAction>
void StateMachine<TOptions>::add(
        SourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>&& t)
{
    using namespace std;
    transition_type* transition = new transition_type(std::move(t));
    transition->source()->pushBackTransition(transition);
}

template <typename TOptions>
void StateMachine<TOptions>::enterStatesInEnterSet(unsigned event)
{
    for (pre_order_iterator iter = begin(); iter != end(); ++iter)
    {
        if (iter->m_flags & state_type::InEnterSet)
        {
            std::cout << "[StateMachine] Enter " << iter->name() << std::endl;
            iter->onEntry(event);
            iter->m_internalActive = true;
        }
    }
}

template <typename TOptions>
void StateMachine<TOptions>::leaveStatesInExitSet(unsigned event)
{
    for (post_order_iterator iter = post_order_begin();
         iter != post_order_end(); ++iter)
    {
        if (iter->m_flags & state_type::InExitSet)
        {
            std::cout << "[StateMachine] Leave " << iter->name() << std::endl;
            iter->m_internalActive = false;
            iter->exitInvoke();
            iter->onExit(event);
        }
    }
}

template <typename TOptions>
void StateMachine<TOptions>::microstep(event_type event)
{
    // 1. Mark the states in the exit set for exit and the target state of the
    //    transition for entry.
    for (transition_type *prev = 0, *transition = m_enabledTransitions;
         transition != 0;
         prev = transition, transition = transition->m_nextInEnabledSet)
    {
        if (!transition->target())
            continue;

        state_type* domain = transitionDomain(transition);

        if (prev)
        {
            // Make sure that no state of the transition domain has been
            // marked for exit. Otherwise, two transitions have an
            // overlapping exit set, which means that the transitions
            // conflict.
            bool conflict = false;
            for (const_pre_order_iterator iter = subtree_cbegin(domain);
                 iter != subtree_cend(domain); ++iter)
            {
                if (iter->m_internalActive
                    && (iter->m_flags & state_type::InExitSet))
                {
                    conflict = true;
                    break;
                }
            }

            // In case of a conflict, we simply ignore this transition but
            // keep the old ones.
            if (conflict)
            {
                prev->m_nextInEnabledSet = transition->m_nextInEnabledSet;
                transition->m_nextInEnabledSet = 0;
                transition = prev;
                continue;
            }
        }

        // As there is no conflict, we can set the exit mark for the states in
        // the transition domain.
        for (pre_order_iterator iter = subtree_begin(domain);
             iter != subtree_end(domain); ++iter)
        {
            if (iter->m_internalActive)
                iter->m_flags |= state_type::InExitSet;
        }

        // Finally, mark the ancestors of the target for entry, too. Note that
        // we cannot mark the children right now, because another transition
        // can target one of this target's descendants.
        state_type* ancestor = transition->target();
        while (ancestor && !(ancestor->m_flags & state_type::InEnterSet))
        {
            ancestor->m_flags |= state_type::InEnterSet;
            ancestor = ancestor->parent();
        }
    }

    // 2. Propagate the entry mark to the children.
    markDescendantsForEntry();

    // 3. Leave the states in the exit set.
    leaveStatesInExitSet(event);

    // 4. Execute the transition's actions.
    for (transition_type* transition = m_enabledTransitions;
         transition != 0;
         transition = transition->m_nextInEnabledSet)
    {
        if (transition->action())
            transition->action()(event);
    }

    // 5. Enter the states in the enter set.
    enterStatesInEnterSet(event);
}

template <typename TOptions>
void StateMachine<TOptions>::markDescendantsForEntry()
{
    for (pre_order_iterator state = begin(); state != end(); ++state)
    {
        if (!(state->m_flags & state_type::InEnterSet))
        {
            state.skipChildren();
            continue;
        }

        if (state->isCompound())
        {
            // Exactly one state of a compound state has to be marked for entry.
            bool childMarked = false;
            for (sibling_iterator child = state.child_begin();
                 child != state.child_end(); ++child)
            {
                if (child->m_flags & state_type::InEnterSet)
                {
                    childMarked = true;
                    break;
                }
            }

            if (!childMarked)
            {
                //! \todo Add the possibility to have an initial state
                state->m_children->m_flags |= state_type::InEnterSet;
            }
        }
        else if (state->isParallel())
        {
            // All child states of a parallel state have to be marked for entry.
            for (sibling_iterator child = state.child_begin();
                 child != state.child_end(); ++child)
            {
                child->m_flags |= state_type::InEnterSet;
            }
        }
    }
}

template <typename TOptions>
void StateMachine<TOptions>::selectTransitions(bool eventless, event_type event)
{
    transition_type** outputIter = &m_enabledTransitions;

    // Loop over the states in post-order. This way, the descendent states are
    // checked before their ancestors.
    for (auto stateIter = post_order_begin(); stateIter != post_order_end();
         ++stateIter)
    {
        if (!stateIter->m_internalActive)
            continue;

        // If a transition in a descendant of a parallel state has already
        // been selected, the parallel state itself and all its ancestors
        // can be skipped.
        if (stateIter->m_flags & state_type::SkipTransitionSelection)
            continue;

        bool foundTransition = false;
        for (auto transitionIter = stateIter->beginTransitions();
             transitionIter != stateIter->endTransitions(); ++transitionIter)
        {
            if (eventless != transitionIter->eventless())
                continue;

            if (!eventless && transitionIter->event() != event)
                continue;

            // If the transition has a guard, it must evaluate to true in order
            // to select the transition. A transition without guard is selected
            // unconditionally.
            if (!transitionIter->guard() || transitionIter->guard()(event))
            {
                *outputIter = &*transitionIter;
                outputIter = &transitionIter->m_nextInEnabledSet;
                foundTransition = true;
                break;
            }
        }

        if (foundTransition)
        {
            // As we have found a transition in this state, there is no need to
            // check the ancestors for a matching transition.
            bool hasParallelAncestor = false;
            state_type* ancestor = stateIter->parent();
            while (ancestor)
            {
                ancestor->m_flags |= state_type::SkipTransitionSelection;
                hasParallelAncestor |= ancestor->isParallel();
                ancestor = ancestor->parent();
            }

            // If none of the ancestors is a parallel state, there is no
            // need to continue scanning the other states. This is because
            // the remaining active states are all ancestors of the current
            // state and no transition in an ancestor is more specific than
            // the one which has been selected right now.
            if (!hasParallelAncestor)
                return;
        }
    }
}


#if 0
template <typename TDerived>
void StateMachine::apply(Visitor<TDerived>& visitor)
{
    state_type* iterator = &m_rootState;
    while (1)
    {
        visitor.doVisit(iterator);
        if (iterator->m_children)
        {
            iterator = iterator->m_children;
        }
        else
        {
            visitor.doLeave(iterator);
            while (iterator->m_nextSibling == 0)
            {
                iterator = iterator->m_parent;
                if (iterator == 0)
                    return;
                visitor.doLeave(iterator);
            }
            iterator = iterator->m_nextSibling;
        }
    }
}

template <typename TDerived>
void Visitor<TDerived>::operator() (StateMachine& machine)
{
    machine.apply(*this);
}

template <typename TDerived>
void ConfigurationVisitor<TDerived>::operator() (StateMachine& machine)
{
    machine.apply(*this);
}
#endif

struct default_options
{
    using event_type = unsigned;
    using event_list_type = std::deque<unsigned>;
    static constexpr bool synchronous_dispatch = false;
};

} // namespace detail

struct SynchronousEventDispatching
{
    static constexpr bool synchronous_dispatch = true;
};

struct AsynchronousEventDispatching
{
    static constexpr bool synchronous_dispatch = false;
};

template <typename TType>
struct EventType
{
    using event_type = TType;
};

template <typename TType>
struct EventListType
{
    using event_list_type = TType;
};

//using namespace detail;
#if 0
template <typename... TOptions>
struct makeStateMachineType
{

};

template <typename... TOptions>
using StateMachine = makeStateMachineType<TOptions...>::type;
#endif

using StateMachine = detail::StateMachine<detail::default_options>;


/* Options:

template <bool TEnable>
struct SendConfigurationChangeNotification { static constexpr bool notifications = TEnable; };

*/

} // namespace statemachine

#endif // STATEMACHINE_STATEMACHINE_HPP
