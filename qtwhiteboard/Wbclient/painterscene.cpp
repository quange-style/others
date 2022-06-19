#include "painterscene.h"
#include <QDebug>

////////////////class SceneLineEdit /////////////////////////////

SceneLineEdit::SceneLineEdit(QWidget *parent):QLineEdit(parent)
{
    connect(this, SIGNAL(textChanged(QString )), this, SLOT(slot_textChange(QString)));
	this->setStyleSheet(QLatin1String("QLineEdit{background-color:transparent;border:1px solid  red;padding-left:2px;}"));
	m_fontSize= 15;
	QColor color=QColor(0xff, 0x0, 0xff, 0xFF);
	setEditPaintInfo(5,color);
}

void SceneLineEdit::setEditPaintInfo(int fontSize,QColor  &color)
{

	/*QPalette palette;
	palette.setColor(QPalette::ButtonText,color);
	palette.setBrush(QPalette::Button,QBrush(QColor(0,0,0,0)));
	this->setPalette(palette);
	QFont font=QFont( "" , fontSize, QFont::Normal);*/

	QPalette palette;
	palette.setColor(QPalette::Text,color);
	palette.setBrush(QPalette::Window,QBrush(QColor(0,255,255,0)));
	fontSize=fontSize+10;

	QFont font=QFont( "" , fontSize, QFont::Normal);
    qDebug() << "fontSize="<<fontSize<<"color="<<color<<"font"<<font;
	if(m_fontSize!=fontSize){
		m_fontSize=fontSize;
		this->setFont(font);
		slot_textChange(this->text());
	}

	this->setPalette(palette);

}


void SceneLineEdit::slot_textChange(QString text)
{
	QFont font("", m_fontSize, QFont::Normal);
	QFontMetrics fontmetr(font);
	int text_width= fontmetr.width(text);
	int text_height= fontmetr.height();

    //qDebug() << "text_width="<<text_width<<"width="<<width();
	if(text_width > (this->width()-10)){
		this->setFixedWidth(text_width+10);
	}


    //qDebug() << "text_height="<<text_height<<"height="<<height();
	if(text_height > this->height()-2){
		this->setFixedHeight(text_height+5);
	}
}



////////////////class PainterScene /////////////////////////////
PainterScene::PainterScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_toolType(tt_Line)
    , m_currentShape(nullptr)
    , m_id(-1),m_lineEdit(nullptr)
{
    //qDebug() << "PainterScene begin";

	m_parent=(QWidget *)parent;
	m_lineWidth=2;
	m_lineColor=QColor(0xff, 0Xf, 0, 0xFF);
	m_fillColor=Qt::transparent;
}

PainterScene::~PainterScene(){}

void PainterScene::setToolType(int type,int lineWidth,QColor &color,QColor fillcolor){
    m_toolType = type;
	m_lineWidth=lineWidth;
	m_lineColor=color;
	m_fillColor=fillcolor;

	if(m_lineEdit){
		if(m_toolType!=tt_Text){
			m_lineEdit->hide();
		}
		m_lineEdit->setEditPaintInfo(lineWidth,color);
	}

    if(m_currentShape != nullptr){
        if(!m_currentShape->isValid()){
            delete m_currentShape;
        }
        m_currentShape = nullptr;//下次事件发生时候就可以做相应处理
    }
}

void PainterScene::onFigureAdded(const QJsonObject &figure){
    qDebug() << "PainterScene::onFigureAdded";
    Shape *item = nullptr;//生成一个即将添加的item
    QJsonObject data = figure.value("data").toObject();
    QJsonArray points = data.value("points").toArray();
    QString strType = figure.value("type").toString();

    int creatorId = figure.value("creator").toInt();
    int localId = figure.value("local_id").toInt();
    int globalId = figure.value("global_id").toInt();

    if(creatorId == m_id){
        auto it = std::find_if(m_shapes.begin(), m_shapes.end()
                               ,[=](Shape *s){
            return (s->creatorId() == creatorId && s->localId() == localId);
        });
        if(it != m_shapes.end()){
            //item已经在场景中，不需要调整
            qDebug() << "Update figure global id";
            (*it)->setGlobalId(globalId);
            return;
        }
    }
#define SET_START_AND_END_POS() \
    QPointF startPos(points[0].toInt(), points[1].toInt());\
    QPointF endPos(points[2].toInt(), points[3].toInt());\
    item->setStartPoint(startPos);\
    item->setEndPoint(endPos);

    if(strType == "line"){
        item = new SLine();
        SET_START_AND_END_POS();
    }
    else if(strType == "oval"){
        item = new SOval();
        SET_START_AND_END_POS();

    }
    else if(strType == "rectangle"){
        item = new SRectangle();
        SET_START_AND_END_POS();
    }
    else if(strType == "triangle"){
        item = new STriangle();
        SET_START_AND_END_POS();

    }
	 else if(strType == "text"){
	 	item = new SText(data.value("text").toString());
		SET_START_AND_END_POS();
    }
    else if(strType == "graffiti"){
        SGraffiti *graffiti= new SGraffiti();
        item = graffiti;
        QPainterPath path;
        int size = points.size();
        path.moveTo(points[0].toInt(), points[1].toInt());//移到起始点，第一个点
        for(int i = 2; i < size; i+=2){
            path.lineTo(points[i].toInt(), points[i+1].toInt());
        }
        graffiti->setPath(path);//将路径传递给graffiti
    }
    else{
        qDebug() << "Unknown figure type!";
        return;
    }

    item->setGlobalId(globalId);
    item->setLocalId(localId);
    item->setStrokeWidth(data.value("line_width").toInt());
    item->setStrokeColor(QColor::fromRgba((unsigned int)data.value("color").toDouble()));
    item->setFillColor(QColor::fromRgba((unsigned int)data.value("fill_color").toDouble()));

    addItem(item);//加入场景中
    m_shapes.push_front(item);//加入链表中
    update();
}

void PainterScene::onFigureDeleted(int id){
    auto it = std::find_if(m_shapes.begin(), m_shapes.end(),
                           [=](Shape*s){
        return s->globalId() == id;
    });
    if(it != m_shapes.end()){
        qDebug() << "PainterScene::onFigureDeleted, global id - " << id;
        removeItem(*it);
        delete *it;
        m_shapes.erase(it);
        update();
    }

}

void PainterScene::onFigureCleared(int ownerId){
    if(ownerId == -1)
    {
        qDebug() << "PainterScene::onFigureCleared, Clear All ";
        clear();
        m_shapes.clear();
		m_lineEdit=nullptr;
        update();
    }
    else{
        auto it = m_shapes.begin();

        while(it != m_shapes.end()){
            if((*it)->creatorId() == ownerId){
                removeItem(*it);
                delete *it;
                it = m_shapes.erase(it);
                qDebug() << "PainterScene::onFigureCleared, Clear one figure of " << ownerId;
                update();//?
            }
            else{
                it++;
            }
        }
    }
}

void PainterScene::undo(){
    if(m_shapes.size()){
        Shape *item = m_shapes.back();
        emit deleteFigureReq(item->globalId());
    }
}

bool PainterScene::trigerText(QPointF pos,QPointF &lastPos){
	//qDebug() << "trigerText pos="<<pos<<"lastPos"<<lastPos;
	//qDebug() << "m_lineEdit="<<m_lineEdit;
	if(!m_lineEdit){
		m_lineEdit=new SceneLineEdit();
		m_lineEdit->setGeometry(pos.x(),pos.y(),90,30);
		this->addWidget(m_lineEdit);
		m_LastPosScene=pos;
		m_lineEdit->setEditPaintInfo(m_lineWidth,m_lineColor);

		/*QPalette palette;
		palette.setColor(QPalette::Text,color);
		QFont font=QFont( "" , 20, QFont::Normal);
		palette.setBrush(QPalette::Window,QBrush(QColor(0,255,255,0)));

		m_lineEdit->setFont(font);
		m_lineEdit->setPalette(palette);*/

		//QColor color=QColor(0xFF, 0xFF, 0xFF, 0xFF);
		//m_lineEdit->setEditPaintInfo(20,color);

	}
	m_lineEdit->setGeometry(pos.x(),pos.y(),90,30);
	//m_lineEdit->setFocusPolicy(Qt::StrongFocus);
	//m_lineEdit->setFocus();
	m_lineEdit->show();
	lastPos=m_LastPosScene;
	//qDebug() << "trigerText pos="<<pos;

	if(m_LastPosScene!=pos){
		m_LastPosScene=pos;
		return true;
	}
	return false;
}

bool PainterScene::drawLineText(QPointF pos,QString &text){
	if(m_lineEdit){
		text=m_lineEdit->text();
		//m_lineEdit->hide();
		m_lineEdit->clear();

		qDebug() << "drawLineText text"<<text;
		if(text.length()>0)
			return true;
		else
			return false;
	}
	return false;
}


void PainterScene::createShape(int toolType,QPointF pos,QString text){
	switch(toolType){
        case tt_Line:
            m_currentShape = new SLine();
            break;
        case tt_Circle:
            m_currentShape = new SOval();

            break;
        case tt_Rectangle:
            m_currentShape = new SRectangle();

            break;
        case tt_Triangle:
            m_currentShape = new STriangle();

            break;
        case tt_Graffiti:
            m_currentShape = new SGraffiti();

            break;
		case tt_Text:
		   m_currentShape = new SText(text);
		   break;

        default:
            return;

        }

		if(m_currentShape == nullptr) return;//异常，不作处理
		m_currentShape->setStrokeColor(m_lineColor);
		m_currentShape->setStrokeWidth(m_lineWidth);
		m_currentShape->setFillColor(m_fillColor);

        addItem(m_currentShape);
        m_currentShape->setCreatorId(m_id);
        m_currentShape->setStartPoint(pos);
}


void PainterScene::addShapeToList(){
	 if(m_currentShape!= nullptr && m_currentShape->isValid()){
        m_shapes.push_front(m_currentShape);//图形保存到列表中
        QJsonObject figure;
        m_currentShape->serialize(figure);//将绘制的Json信息保存在figure中
        figure.insert("creator", QJsonValue(m_id));
        figure.insert("local_id", QJsonValue(m_currentShape->localId()));
        emit addFigureReq(figure);
    }
    else{
        removeItem(m_currentShape);
        delete m_currentShape;
    }
    m_currentShape = nullptr;
}

void PainterScene::mousePressEvent(QGraphicsSceneMouseEvent *ev){
	//qDebug() << "mousePressEvent "<<m_toolType;

    QGraphicsScene::mousePressEvent(ev);
    if(ev->button() != Qt::LeftButton) return;//只处理鼠标左键
    if(!ev->isAccepted() ){//父类还没处理
    	if(m_toolType!=tt_Text){
			createShape(m_toolType,ev->scenePos());
		}

    }
}

void PainterScene::mouseMoveEvent(QGraphicsSceneMouseEvent *ev){
	//qDebug() << "mouseReleaseEvent "<<m_toolType;


    QGraphicsScene::mouseMoveEvent(ev);
    if(!ev->isAccepted() && m_currentShape && m_toolType!=tt_Text ){		
        m_currentShape->setEndPoint(ev->scenePos());
        update();
    }
	
}

void PainterScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev){
	//qDebug() << "mouseReleaseEvent "<<m_toolType;
    QGraphicsScene::mouseReleaseEvent(ev);
    if(ev->button() != Qt::LeftButton) return;//只处理鼠标左键
    if(!ev->isAccepted() ){
		if(m_toolType!=tt_Text){		 	
			addShapeToList();
		}
	   else if(m_toolType==tt_Text){
	   	   QString text;
		   QPointF lastPos;

		   if(trigerText(ev->scenePos(),lastPos) && drawLineText(lastPos,text)){
				createShape(tt_Text,lastPos,text);
				update();
				addShapeToList();
		   }
	    }
    }
}
