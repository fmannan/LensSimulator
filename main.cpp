#include "mainwindow.h"
#include <QApplication>
#include <ray.h>
#include <QDebug>

void testRayIntersection()
{
    Ray r1, r2;

    glm::vec3 ip;
    Ray::RayIntersectionType status;
    glm::vec3 dir, n;

    qDebug() << "testing YZ plane";

    r1 = Ray(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    r2 = Ray(glm::vec3(0.0f, 2.0f, 2.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    status = r1.intersection(r2, ip);
    qDebug() << status;
    qDebug() << ip.x << " , " << ip.y << " , " << ip.z;


    r1 = Ray(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    r2 = Ray(glm::vec3(10.0f, 2.0f, 2.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    status = r1.intersection(r2, ip);
    qDebug() << status;
    qDebug() << ip.x << " , " << ip.y << " , " << ip.z;

    r1 = Ray(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, -1.0f, 1.0f));
    r2 = Ray(glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(0.0f, 1.0f, 1.0f));
    status = r1.intersection(r2, ip);
    qDebug() << status;
    qDebug() << ip.x << " , " << ip.y << " , " << ip.z;

    r1 = Ray(glm::vec3(0  ,  17.0215  ,  19.4149), glm::vec3(0  ,  -0.213445  ,  0.976955));
    r2 = Ray(glm::vec3(0  ,  -17.0215  ,  19.4149), glm::vec3(0  ,  0.213445  ,  0.976955));
    status = r1.intersection(r2, ip);
    qDebug() << status;
    qDebug() << ip.x << " , " << ip.y << " , " << ip.z;
    dir = ip - r1.origin();
    n = glm::cross(dir, r1.direction());
    qDebug() << glm::dot(n, n);

    qDebug() << "testing XY plane";

    r1 = Ray(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    r2 = Ray(glm::vec3(2.0f, 2.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    status = r1.intersection(r2, ip);
    qDebug() << status;
    qDebug() << ip.x << " , " << ip.y << " , " << ip.z;

    r1 = Ray(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    r2 = Ray(glm::vec3(2.0f, 2.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    status = r1.intersection(r2, ip);
    qDebug() << status;
    qDebug() << ip.x << " , " << ip.y << " , " << ip.z;

    r1 = Ray(glm::vec3(0.0f, 2.0f, 1.0f), glm::vec3(1.0f, -1.0f, 0.0f));
    r2 = Ray(glm::vec3(0.0f, -2.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.0f));
    status = r1.intersection(r2, ip);
    qDebug() << status;
    qDebug() << ip.x << " , " << ip.y << " , " << ip.z;

    r1 = Ray(glm::vec3(0.0f, 2.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.0f));
    r2 = Ray(glm::vec3(0.0f, -2.0f, 1.0f), glm::vec3(1.0f, -1.0f, 0.0f));
    status = r1.intersection(r2, ip);
    qDebug() << status;
    qDebug() << ip.x << " , " << ip.y << " , " << ip.z;

    r1 = Ray(glm::vec3(0.0f, 2.0f, 2.0f), glm::vec3(1.0f, 1.0f, 0.0f));
    r2 = Ray(glm::vec3(0.0f, -2.0f, 1.0f), glm::vec3(1.0f, -1.0f, 0.0f));
    status = r1.intersection(r2, ip);
    qDebug() << status;
    qDebug() << ip.x << " , " << ip.y << " , " << ip.z;

    r1 = Ray(glm::vec3(0.0f, 2.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.0f));
    r2 = Ray(glm::vec3(0.0f, -2.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.0f));
    status = r1.intersection(r2, ip);
    qDebug() << status;
    qDebug() << ip.x << " , " << ip.y << " , " << ip.z;


}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    // perform ray intersection test
    testRayIntersection();

    return a.exec();
}
