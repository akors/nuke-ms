// refcounter.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2009  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, version 3 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef REFCOUNTER_HPP
#define REFCOUNTER_HPP

#include <boost/function.hpp>

#ifdef REFCOUNTER_MULTITHREADED
#   include <boost/thread/locks.hpp>
#   include <boost/thread/mutex.hpp>

#   define MUTEX_IF_MULTITHREADED() boost::mutex reference_mutex;
#   define LOCK_IF_MULTITHREADED() boost::lock_guard<boost::mutex> \
        lock(reference_mutex);

#else

#   define MUTEX_IF_MULTITHREADED()
#   define LOCK_IF_MULTITHREADED()

#endif

/** Count references to the current object.
* This class is used to maintain track of the references to the current object.
* This can be usefull if the lifetime of an object is dependent on the
* references held to it.
* In short, to ensure that the object is alive as long as there are references
* to it.
* To do so, derive the Class you want to have reference counted from this class
* template. When you need a counted reference, create a CountedReference object
* with *this as constructor argument, and make copies of it at will.
* Construction of CountedReference objects will increase the reference count,
* destruction will decrease the reference count.
*
* When the reference count reaches zero by decreasing, the action specified in
* the constructor will be executed.
*
* @tparam The type of the deriving class
*/
template <typename ReferencedType>
class ReferenceCounter
{
    // mutex that ensures thread safe acces to the reference count
    MUTEX_IF_MULTITHREADED()

    /**< Action that will be executed when the reference count reaches zero */
    boost::function<void ()> action;

    unsigned reference_count; /** Number of counted references */

public:

    /** Counted Reference.
    * This class represents a reference to an object and registers itself
    * within a ReferenceCounter object.
    * Use it to create a tracked reference.
    *
    * You can retreive the reference to the object by calling the
    * cast to ReferenceCounter operator or by the ref() function.
    */
    class CountedReference
    {
        /** Reference to the counter, so we can register *this */
        ReferenceCounter& referencecounter;

    public:
        /** Constructor.
        * Increases reference count in associated ReferenceCounter object by
        * calling the ReferenceCounter::increaseRefCount() function.
        *
        * @param _referencecounter The ReferenceCounter this reference shall be
        * associated with.
        * @see ReferenceCounter::increaseRefCount()
        */
        CountedReference(ReferenceCounter& _referencecounter)  throw()
            : referencecounter(_referencecounter)
        { referencecounter.increaseRefCount(); }

        /** Copy Constructor.
        * Increases reference count in associated ReferenceCounter object by
        * calling the ReferenceCounter::increaseRefCount() function.
        *
        * @param other The counted reference object. The ReferenceCounter of
        * other will be used as this ReferenceCounter object.
        * @see ReferenceCounter::increaseRefCount()
        */
        CountedReference(const CountedReference& other) throw()
            : referencecounter(other.referencecounter)
        { referencecounter.increaseRefCount(); }

        /** Denstructor.
        * Decreases reference count in associated ReferenceCounter object by
        * calling the ReferenceCounter::increaseRefCount() function.
        *
        * If the reference_count reaches zero after decreasing, and an action
        * was specified in the constructor of the ReferenceCounter object,
        * the action is executed.
        *
        * @see ReferenceCounter::decreaseRefCount()
        */
        ~CountedReference() throw()
        { referencecounter.decreaseRefCount(); }

        /** Cast operator.
        * Returns a reference to the referenced type.
        * @return a reference to the referenced type.
        */
        operator ReferencedType& ()
        { return static_cast<ReferencedType&>(referencecounter); }

        /** Returns a reference to the referenced type.
        * @return a reference to the referenced type.
        */
        ReferencedType& ref()
        { return operator ReferencedType& (); }
    };

protected:
    /** Constructor.
    * Ininitialize the reference counter.
    * If you want an action to be executed when the reference count reaches
    * zero, specify it in the constructor.
    *
    * @param Action that will be executed when the reference count reaches zero
    */
    ReferenceCounter(
        boost::function<void ()> _action = boost::function<void ()>()
    ) throw()
        : action(_action), reference_count(0u)
    {}

    /** Return reference count.
    * @return Number of counted references
    */
    inline unsigned getRefCount() const
    { return reference_count; }

private:
    // CountedReference is our friend
    friend class CountedReference;

    /** No copy construction allowed */
    ReferenceCounter(const ReferenceCounter&);

    /** Increase reference count.
    * To be called only by the constructor of a CountedReference object.
    *
    * @post reference_count increased by one
    * @note In multithreaded builds this function locks a mutex during execution
    */
    inline void increaseRefCount ()
    {
        LOCK_IF_MULTITHREADED()

        reference_count++;
    }

    /** Decrease reference count
    * To be called only by the denstructor of a CountedReference object.
    *
    * If the reference_count reaches zero after decreasing, and an action
    * was specified in the constructor, the action is executed.
    *
    * @post reference_count decreased by one
    * @note In multithreaded builds this function locks a mutex during execution
    */
    inline void decreaseRefCount ()
    {
        LOCK_IF_MULTITHREADED()

        reference_count--;

        if (reference_count <= 0u && action)
            action();

        assert (reference_count >= 0);
    }

};


#endif // ifndef REFCOUNTER_HPP
