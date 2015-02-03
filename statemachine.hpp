#ifndef FSM11_STATEMACHINE_HPP
#define FSM11_STATEMACHINE_HPP

#include "state.hpp"
#include "statemachine_fwd.hpp"
#include "transition.hpp"

#include "detail/callbacks.hpp"
#include "detail/eventdispatcher.hpp"
#include "detail/multithreading.hpp"
#include "detail/options.hpp"
#include "detail/storage.hpp"

//#include "/home/manuel/code/weos/src/scopeguard.hpp"


#include <deque>
#include <iterator>
#include <mutex>
#include <type_traits>

namespace fsm11
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










//! \brief A finite state machine.
//!
//! The StateMachine implements the logic for a finite state machine.
//!
//! Every state machine has an implicit root state. All top-level states of
//! a state machine, must be added as child of this root state.
template <typename TOptions>
class StateMachine :
        public get_multithreading<TOptions>::type,
        public get_dispatcher<TOptions>::type,
        public get_configuration_change_callbacks<TOptions>::type,
        public get_event_callbacks<TOptions>::type,
        public get_state_callbacks<TOptions>::type,
        public get_storage<TOptions>::type,
        public State<TOptions>
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
        : state_type("(StateMachine)")
    {
        state_type::m_stateMachine = this;
        //m_rootState.m_stateMachine = this;
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
    //! \p spec.
    template <typename TState, typename TGuard, typename TAction>
    void add(SourceNoEventGuardActionTarget<TState, TGuard, TAction>&& t);

    //! \brief Adds a transition.
    template <typename TState, typename TEvent, typename TGuard,
              typename TAction>
    inline
    StateMachine& operator<<(
            SourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>&& t)
    {
        add(t);
    }


#if 0
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
#endif
private:
    //! A list of events which have to be handled by the event loop.
    event_list_type m_eventList;

    //! The implicit root state.
    //state_type m_rootState;


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
template <typename TState, typename TGuard, typename TAction>
void StateMachine<TOptions>::add(
        SourceNoEventGuardActionTarget<TState, TGuard, TAction>&& t)
{
    using namespace std;
    transition_type* transition = new transition_type(std::move(t));
    transition->source()->pushBackTransition(transition);
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
    static constexpr bool synchronous_dispatch = true;
    static constexpr bool multithreading_enable = false;

    static constexpr bool event_callbacks_enable = false;
    static constexpr bool configuration_change_callbacks_enable = false;
    static constexpr bool state_callbacks_enable = false;
};

} // namespace detail



struct SynchronousEventDispatching
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool synchronous_dispatch = true;
    };
    //! \endcond
};

struct AsynchronousEventDispatching
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool synchronous_dispatch = false;
    };
    //! \endcond
};

template <typename TType>
struct EventType
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        using event_type = TType;
    };
    //! \endcond
};

template <typename TType>
struct EventListType
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        using event_list_type = TType;
    };
    //! \endcond
};

template <bool TEnable>
struct EnableEventCallbacks
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool event_callbacks_enable = TEnable;
    };
    //! \endcond
};

template <bool TEnable>
struct EnableConfigurationChangeCallbacks
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool configuration_change_callbacks_enable = TEnable;
    };
    //! \endcond
};

template <bool TEnable>
struct EnableStateCallbacks
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool state_callbacks_enable = TEnable;
    };
    //! \endcond
};



template <typename... TOptions>
struct makeStateMachineType
{
private:
    using packed_options = typename detail::pack_options<detail::default_options,
                                                         TOptions...>::type;
public:
    using type = detail::StateMachine<packed_options>;
};



template <typename... TOptions>
using StateMachine = typename makeStateMachineType<TOptions...>::type;



#if DOXYGEN

template <typename... TOptions>
class Statemachine
{
public:
    //! The type of the state machine.
    using type = StateMachine<TOptions...>;
    using event_type = typename TOptions::event_type;
    using event_list_type = typename TOptions::event_list_type;
    using state_type = State<TOptions>;
    using transition_type = Transition<TOptions>;


    //! Adds another \p event to the state machine.
    void addEvent(event_type event);

    //! Starts the state machine.
    void start();

    //! Starts the state machine.
    //!
    //! Starts the state machine. The attributes \p attrs are passed to the
    //! thread running the event loop.
    //!
    //! \note This overload is only available, if asynchronous event
    //! dispatching has been enabled.
    void start(const weos::thread::attributes& attrs);

    void stop();

    bool running() const;



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


    //! Loads an element from the storage.
    //!
    //! \p T is the type of the \p TIndex-th element in the storage.
    //!
    //! \note This function is only available, if a storage has been specified.
    template <std::size_t TIndex>
    const T& load() const;

    //! Sets an element in the storage.
    //!
    //! Sets the \p TIndex-th element in the storage to the given \p value.
    //!
    //! \note This function is only available, if a storage has been specified.
    template <std::size_t TIndex, typename TType>
    void store(TType&& value);
};

#endif // DOXYGEN

} // namespace fsm11

#endif // FSM11_STATEMACHINE_HPP
