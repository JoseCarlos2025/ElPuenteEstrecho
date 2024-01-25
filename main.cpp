#include <QThread>
#include <QMutex>
#include <QDebug>
#include <QList>
#include <QSemaphore>
#include <QWaitCondition>

class Puente {
public:
    Puente() : vehiclesOnBridge(0), maxVehicles(3), bridgeMutex(), condition() {}

    void entrarAlPuente(QString sentido) {
        QMutexLocker locker(&bridgeMutex);

        while ((sentido == "Izquierda" && vehiclesRight > 0) ||
               (sentido == "Derecha" && vehiclesLeft > 0) ||
               (vehiclesOnBridge >= maxVehicles)) {
            qDebug() << "Vehículo esperando para entrar al puente en sentido" << sentido;
            condition.wait(&bridgeMutex);
        }

        vehiclesOnBridge++;
        qDebug() << "Vehículo entrando al puente en sentido" << sentido << "Total en el puente:" << vehiclesOnBridge;

        if (sentido == "Izquierda") {
            vehiclesLeft++;
        } else {
            vehiclesRight++;
        }
    }

    void salirDelPuente(QString sentido) {
        QMutexLocker locker(&bridgeMutex);

        qDebug() << "Vehículo saliendo del puente en sentido" << sentido;

        vehiclesOnBridge--;

        if (sentido == "Izquierda") {
            vehiclesLeft--;
        } else {
            vehiclesRight--;
        }

        condition.wakeAll();
    }

private:
    int vehiclesOnBridge;
    int maxVehicles;
    QMutex bridgeMutex;
    QWaitCondition condition;
    int vehiclesLeft;
    int vehiclesRight;
};

class Vehiculo : public QThread {
public:
    Vehiculo(QString sentido, Puente* puente) : sentido(sentido), puente(puente) {}

    void llegarAlPuente() {
        qDebug() << "Vehículo llegando al puente en sentido" << sentido;
        puente->entrarAlPuente(sentido);
        QThread::sleep(1);
        puente->salirDelPuente(sentido);
    }

    void run() override {
        llegarAlPuente();
    }

private:
    QString sentido;
    Puente* puente;
};

int main() {

    Puente puente;
    QList<Vehiculo*> vehiculos;

    for (int i = 0; i < 10; ++i) {
        Vehiculo* vehiculoLeft = new Vehiculo("Izquierda", &puente);
        vehiculoLeft->start();
        vehiculos.append(vehiculoLeft);

        Vehiculo* vehiculoRight = new Vehiculo("Derecha", &puente);
        vehiculoRight->start();
        vehiculos.append(vehiculoRight);
    }

    for (Vehiculo* vehiculo : vehiculos) {
        vehiculo->wait();
        delete vehiculo;
    }

    return 0;
}
