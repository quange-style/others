#ifndef PAINTERSCENE_H
#define PAINTERSCENE_H

#include <QGraphicsScene>
#include "shape.h"
#include <QJsonObject>
#include <list>

class SceneLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	explicit SceneLineEdit(QWidget *parent=0);

	void setEditPaintInfo(int fontSize,QColor &color);

public slots:
	void slot_textChange(QString text);

protected:
	int m_fontSize;
};


class PainterScene: public QGraphicsScene{
    Q_OBJECT
public:
    PainterScene(QObject *parent = 0);
    ~PainterScene();

    void setToolType(int type,int lineWidth,QColor &color,QColor fillcolor=Qt::transparent);

    void setUserId(int id){m_id = id ;}//提供对外接口来设置id
    void onFigureAdded(const QJsonObject &figure);
    void onFigureDeleted(int id);
    void onFigureCleared(int OwnerId);
    void undo();

	void addShapeToList();
	void createShape(int toolType,QPointF pos,QString text="");

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;

	bool drawLineText(QPointF pos,QString &text);
	bool trigerText(QPointF pos,QPointF &lastPos);

signals:
    void addFigureReq(const QJsonObject &figure);//绘制完一个图形发射信号给服务器
    void deleteFigureReq(int id);
    void clearFigureReq(int OwnerId);

protected:

    int m_toolType;
	float m_lineWidth;//线段宽度
    QColor m_lineColor;//线段颜色
    QColor m_fillColor;//填充颜色

    int m_id;
    Shape *m_currentShape;
    std::list<Shape*> m_shapes;//保存所有图形

	SceneLineEdit * m_lineEdit;//文本输入框
	QWidget *m_parent;
    QPointF m_LastPosScene;

};
#endif // PAINTERSCENE_H
