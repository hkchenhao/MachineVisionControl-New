#ifndef WIDGETSTYLE_H
#define WIDGETSTYLE_H
#include <QPainter>
#include <QProxyStyle>
#include <QStyleOptionTab>

// TabWidget样式表
class CustomTabStyle : public QProxyStyle
{
public:
    QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
    {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (type == QStyle::CT_TabBarTab)
        {
            s.transpose();
            s.rwidth() = 170; // 设置每个tabBar中item的大小
            s.rheight() = 50;
        }
        return s;
    }
    void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
    {
        if (element == CE_TabBarTabLabel)
        {
            if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option))
            {
                QRect allRect = tab->rect;
                if (tab->state & QStyle::State_Selected)
                {
                    painter->save();
                    painter->setPen(0x89CFFF);
                    painter->setBrush(QBrush(0x89CFFF));
                    painter->drawRect(allRect.adjusted(0, 0, 0, 0));
                    painter->restore();
                }
                QTextOption option;
                option.setAlignment(Qt::AlignCenter);
                if (tab->state & QStyle::State_Selected)
                    painter->setPen(0xF8FCFF);
                else
                    painter->setPen(0x5D5D5D);
                painter->drawText(allRect, tab->text, option);
                return;
            }
        }
        if (element == CE_TabBarTab)
            QProxyStyle::drawControl(element, option, painter, widget);
    }
};

#endif
