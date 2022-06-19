#include "mainwindow.h"
#include <QtWidgets>
#include "checkbox.h"
#include "painterscene.h"
#include "painterview.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_scene(nullptr)
    , m_conn(nullptr), m_toolBar(nullptr), m_nameEdit(nullptr), m_statusEdit(nullptr)
    , m_hostEdit(nullptr), send_message(nullptr)
{
    prepareJoinUI();
}

MainWindow::~MainWindow()
{

}

void MainWindow::prepareJoinUI(){
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);
    QLabel *namelabel = new QLabel("Input Your Name:");
    layout->addWidget(namelabel);

    m_nameEdit = new QLineEdit("nana");
    layout->addWidget(m_nameEdit);

    QLabel *hostlabel = new QLabel("Input the Server Ip:");
    layout->addWidget(hostlabel);

    m_hostEdit = new QLineEdit("127.0.0.1");
    layout->addWidget(m_hostEdit);

    auto btn = new QPushButton("Join");
    connect(btn, SIGNAL(clicked(bool)), this, SLOT(onJoinButtonClicked()));
    layout->addWidget(btn);

    layout->addStretch(1);

    setCentralWidget(widget);

}

void MainWindow::preparePainterUI(){
    if(!m_toolBar)
    {
        QToolBar *toolbar = addToolBar("Figure Type");
        QActionGroup *actionGroup = new QActionGroup(toolbar);//加到一个组中，可以设定选择或者互斥
        m_toolBar = toolbar;
        toolbar->setOrientation(Qt::Vertical);
        QAction *action = toolbar->addAction(QIcon(":/res/line.png"),
                                             "Draw a Line",
                                             this,SLOT(onDrawLineAct()));

        action->setCheckable(true);//可选中的，能有选中状态
        action->setChecked(true);//已经选中，默认选中
        action->setActionGroup(actionGroup);//关联到actiongroup

        action = toolbar->addAction(QIcon(":/res/rectangle.png"),
                                    "Draw a Rectangle",
                                    this,
                                    SLOT(onDrawRectangleAct()));
        action->setCheckable(true);
        action->setActionGroup(actionGroup);

        action = toolbar->addAction(QIcon(":/res/circle.png"),
                                    "Draw a circle",
                                    this,
                                    SLOT(onDrawCircleAct()));
        action->setCheckable(true);
        action->setActionGroup(actionGroup);

        action = toolbar->addAction(QIcon(":/res/triangle.png"),
                                    "Draw a triangle",
                                    this,
                                    SLOT(onDrawTriangleAct()));
        action->setCheckable(true);
        action->setActionGroup(actionGroup);

        action = toolbar->addAction(QIcon(":/res/pencil.png"),
                                    "Draw free",
                                    this,
                                    SLOT(onDrawFreeAct()));
        action->setCheckable(true);
        action->setActionGroup(actionGroup);
        //删除上一个
        toolbar->addSeparator();//分隔符

        action = toolbar->addAction(QIcon(":/res/return.png"), "Deleted lastest added figure"
                                    ,this, SLOT(onUndo()));
        action->setActionGroup(actionGroup);
        //清除
        action = toolbar->addAction(QIcon(":/res/clearall.png"), "Clear All"
                                    ,this, SLOT(onClearAll()));
        action->setActionGroup(actionGroup);

    }
    else{
        addToolBar(m_toolBar);
    }
    m_scene = new PainterScene();
    m_scene->setSceneRect(0,0,800,600);//wcq add 2022-1-18
    auto *view = new PainterView(m_scene);//将view与scene关联,场景左上角一直与视图左上角重合

    connect(m_scene, SIGNAL(addFigureReq(QJsonObject)),
            this, SLOT(onAddFigureReq(QJsonObject)));
    connect(m_scene, SIGNAL(deleteFigureReq(int)),
            this, SLOT(onDeleteFigureReq(int)));
    connect(m_scene, SIGNAL(clearFigureReq(int)),
            this, SLOT(onClearFigureReq(int)));

    setCentralWidget(view);

    QToolBar *toolbar1 = addToolBar("Widgets");
    //Widget聊天界面
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);

    m_statusEdit = new QTextEdit();//显示状态编辑框
    m_statusEdit->setReadOnly(true);//设置为只读
    layout->addWidget(m_statusEdit, 1);//加上伸缩参数，拉伸时候随窗口一起变化

    QLabel *message = new QLabel("Input The Message:");
    layout->addWidget(message);
    send_message = new QLineEdit("message");
    layout->addWidget(send_message);
    auto sendBtn = new QPushButton("Send");//Send按钮
    connect(sendBtn, SIGNAL(clicked(bool)), this, SLOT(ClicktoSend()));
    layout->addWidget(sendBtn);



    layout->addStretch(1);


    toolbar1->addWidget(widget);



}

void MainWindow::ClicktoSend(){
    if(m_conn && m_conn->state() == QAbstractSocket::ConnectedState){

        QJsonDocument doc;
        QJsonObject root;
        root.insert("type", QJsonValue("chat"));
        root.insert("name", QJsonValue(name_saver));
        root.insert("message", QJsonValue(send_message->text()));
        doc.setObject(root);
        QByteArray json = doc.toJson(QJsonDocument::Compact);
        json.append("\n");
        m_conn->write(json);
        m_conn->flush();
    }
}

void MainWindow::onUserChated(QString mb_name,QString msg){
    qDebug() << __FUNCTION__ ;
    m_statusEdit->append(QString("%1 speaked: %2\n").arg(mb_name).arg(msg));
}
//
void MainWindow::onDrawLineAct(){
    m_scene->setToolType(tt_Line);
}

void MainWindow::onDrawRectangleAct(){
    m_scene->setToolType(tt_Rectangle);
}

void MainWindow::onDrawCircleAct(){
    m_scene->setToolType(tt_Circle);
}

void MainWindow::onDrawTriangleAct(){
    m_scene->setToolType(tt_Triangle);
}

void MainWindow::onDrawFreeAct(){
    m_scene->setToolType(tt_Graffiti);
}

void MainWindow::onUndo(){
    m_scene->undo();
}

void MainWindow::onClearAll(){
    if(m_conn) m_conn->ClearFigures(-1);
}


void MainWindow::onJoinButtonClicked(){
    if(!m_conn){
        m_conn = new WbConnect(this);//创建连接去连接服务器
        QString strName = m_nameEdit->text();
        QString strHost = m_hostEdit->text();
        connect(m_conn, SIGNAL(joined(QString,int)),
                this, SLOT(onJoined(QString,int)));
        connect(m_conn, SIGNAL(someoneleft(QString,int)),
                this, SLOT(onSomeoneleft(QString,int)));
        connect(m_conn, SIGNAL(figureAdded(QJsonObject)),
                this, SLOT(onFigureAdded(QJsonObject)));
        connect(m_conn, SIGNAL(figureDeleted(int)),
                this, SLOT(onFigureDeleted(int)));
        connect(m_conn, SIGNAL(figureCleared(int)),
                this, SLOT(onFigureCleared(int)));
        connect(m_conn, SIGNAL(UserChated(QString,QString))
                ,this, SLOT(onUserChated(QString,QString)));
        m_conn->join(strName, strHost, 1996);
    }
}

void MainWindow::onJoined(QString m_name, int id){
    if(id == m_conn->id()){//自己加入聊天室成功，准备preparePainterUI
        name_saver = m_nameEdit->text();
        m_nameEdit = nullptr;
        preparePainterUI();
        m_scene->setUserId(id);
    }
    else{
        //To do
    }
}


void MainWindow::onSomeoneleft(QString name, int id){
    if(id == m_conn->id()){//自己退出成功，preparePainterUI重置，移除toolBar，调用prepareJoinUI
        m_scene = nullptr;
        removeToolBar(m_toolBar);
        prepareJoinUI();
    }
    else{
        //To do
    }
}

void MainWindow::onFigureAdded(const QJsonObject &figure){
    m_scene->onFigureAdded(figure);//让scene来处理
}

void MainWindow::onFigureDeleted(int id){
    m_scene->onFigureDeleted(id);
}

void MainWindow::onFigureCleared(int OwnerId){
    m_scene->onFigureCleared(OwnerId);
}

void MainWindow::onErrorOccurred(const QString &desc){
    prepareJoinUI();
    //若出错，重置
    if(m_conn){
        m_conn->deleteLater();
        m_conn = nullptr;
    }
}

void MainWindow::onAddFigureReq(const QJsonObject &figure){//MainWindow桥接网络和UI
    if(m_conn) m_conn->AddFigure(figure);//直接转发给wbconnect(服务器）
}

void MainWindow::onDeleteFigureReq(int id){
    if(m_conn) m_conn->DeleteFigure(id);
}

void MainWindow::onClearFigureReq(int ownerId){
    if(m_conn) m_conn->ClearFigures(ownerId);
}
