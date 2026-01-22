#include <QApplication>
#include "simulator.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application information
    app.setApplicationName("InkClock Simulator");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("InkClock");
    app.setOrganizationDomain("inkclock.org");
    
    // Create and show the simulator main window
    Simulator simulator;
    simulator.show();
    
    return app.exec();
}