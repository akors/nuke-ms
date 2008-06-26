#include <iostream>

// basic interface for components that can be "controlled"
// equal to a base class
template <typename ControllerT>
class Controllable
{
    ControllerT* controller;
    
public:
    Controllable(ControllerT* _controller)
        : controller(_controller)
    {}

};

// contract on what Component 1 has to do.
// equal to an abstract base class with pure virtual members
template <typename ControllerT, typename DerivedT>
class AbstractComponent1 : Controllable<ControllerT>
{

public:
    AbstractComponent1(ControllerT* ctrler)
        : Controllable<ControllerT>(ctrler)
    {}

    void doSometing()
    {
        static_cast<DerivedT*>(this)->doSomething();
    }   

};

// contract on what Component 2 has to do.
// equal to an abstract base class with pure virtual members
template <typename ControllerT, typename DerivedT>
class AbstractComponent2 : Controllable<ControllerT>
{

public:
    AbstractComponent2(ControllerT* ctrler)
        : Controllable<ControllerT>(ctrler)
    {}

    void doSometingElse()
    {
        static_cast<DerivedT*>(this)->doSomething();
    }   
};

// That's the guy who knows how to do work and invoke the components correctly
template <template<typename> class Component1, 
            template<typename> class Component2>
class Controller
{
    typedef Controller self_t;
    
    Component1<self_t>* comp1_ptr;
    Component2<self_t>* comp2_ptr;
public:
    Controller()
    {
        comp1_ptr = new Component1<self_t>(this);
        comp2_ptr = new Component2<self_t>(this);
    }

    void doWork()
    {
        comp1_ptr->doSomething();
        comp2_ptr->doSomethingElse();
    }    
    
    ~Controller()
    {
        delete comp1_ptr;
        delete comp2_ptr;
    }
};


///////////////////////////////////////////////////////////////////////////////
// The rest would be in seperate files


template <typename ControllerT>
class Component1Implementation 
    : public AbstractComponent1<ControllerT, Component1Implementation<ControllerT> >
{
public:
    Component1Implementation(ControllerT* controller)
        : AbstractComponent1<ControllerT, 
                                Component1Implementation<ControllerT> >
                                (controller)
    {}

    void doSomething()
    { std::cout<<"This is ComponentImplementation, number 1\n"; }
};

template <typename ControllerT>
class Component2Implementation : 
    public AbstractComponent2<ControllerT, Component2Implementation<ControllerT> >
{
public:
    Component2Implementation(ControllerT* controller)
        : AbstractComponent2<ControllerT, 
                                Component2Implementation<ControllerT> >
                                (controller)
    {}

    void doSomethingElse()
    { std::cout<<"This is ComponentImplementation, number 2\n"; }
};





int main()
{
    Controller<Component1Implementation , Component2Implementation> controller;

    controller.doWork();
}
