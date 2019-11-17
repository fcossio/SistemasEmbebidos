#ifndef MAIN_1PRESENTER_HPP
#define MAIN_1PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Main_1View;

class Main_1Presenter : public touchgfx::Presenter, public ModelListener
{
public:
    Main_1Presenter(Main_1View& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~Main_1Presenter() {};

private:
    Main_1Presenter();

    Main_1View& view;
};


#endif // MAIN_1PRESENTER_HPP
