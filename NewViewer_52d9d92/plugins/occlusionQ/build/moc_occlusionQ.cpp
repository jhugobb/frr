/****************************************************************************
** Meta object code from reading C++ file 'occlusionQ.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../occlusionQ.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qplugin.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'occlusionQ.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_OccQ_t {
    QByteArrayData data[3];
    char stringdata0[16];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_OccQ_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_OccQ_t qt_meta_stringdata_OccQ = {
    {
QT_MOC_LITERAL(0, 0, 4), // "OccQ"
QT_MOC_LITERAL(1, 5, 9), // "updateFPS"
QT_MOC_LITERAL(2, 15, 0) // ""

    },
    "OccQ\0updateFPS\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OccQ[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void OccQ::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<OccQ *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->updateFPS(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject OccQ::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_OccQ.data,
    qt_meta_data_OccQ,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *OccQ::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OccQ::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_OccQ.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Plugin"))
        return static_cast< Plugin*>(this);
    if (!strcmp(_clname, "edu.upc.fib.graug.Plugin/1.0"))
        return static_cast< Plugin*>(this);
    return QObject::qt_metacast(_clname);
}

int OccQ::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}

QT_PLUGIN_METADATA_SECTION
static constexpr unsigned char qt_pluginMetaData[] = {
    'Q', 'T', 'M', 'E', 'T', 'A', 'D', 'A', 'T', 'A', ' ', '!',
    // metadata version, Qt version, architectural requirements
    0, QT_VERSION_MAJOR, QT_VERSION_MINOR, qPluginArchRequirements(),
    0xbf, 
    // "IID"
    0x02,  0x66,  'P',  'l',  'u',  'g',  'i',  'n', 
    // "className"
    0x03,  0x64,  'O',  'c',  'c',  'Q', 
    0xff, 
};
QT_MOC_EXPORT_PLUGIN(OccQ, OccQ)

QT_WARNING_POP
QT_END_MOC_NAMESPACE
