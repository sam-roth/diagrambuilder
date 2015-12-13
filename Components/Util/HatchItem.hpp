#pragma once
/**
 * @date   Jul 15, 2013
 * @author Sam Roth <>
 */


namespace dbuilder {

class HatchItem: public QGraphicsLineItem
{
	QGraphicsTextItem *label;
public:
	HatchItem(QGraphicsItem *parent=0)
	: QGraphicsLineItem(0, 0, 10, 10, parent)
	, label(new QGraphicsTextItem(this))
	{
		auto p = pen();
		p.setWidth(1);
		setPen(p);

		auto f = label->font();
		f.setFamily("Helvetica");
		f.setPointSizeF(f.pointSizeF() * 0.75);
		f.setBold(true);
		label->setFont(f);
	}

	void setText(const QString &text)
	{
		QRectF lineRect(line().p1(), line().p2());
		label->setPlainText(text);
		label->setPos(lineRect.center().x() - label->boundingRect().center().x(),
		              lineRect.bottom());
	}

	QString text()
	{
		return label->toPlainText();
	}

};

}  // namespace dbuilder
