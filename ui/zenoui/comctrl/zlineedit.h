#ifndef __ZLINEEDIT_H__
#define __ZLINEEDIT_H__

#include <QtWidgets>

class ZNumSlider;

class ZLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit ZLineEdit(QWidget* parent = nullptr);
    explicit ZLineEdit(const QString& text, QWidget* parent = nullptr);
    void setNumSlider(const QVector<qreal>& steps);
    void setShowingSlider(bool bShow);

protected:
    bool event(QEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void popupSlider();

    QVector<qreal> m_steps;
    ZNumSlider* m_pSlider;
    bool m_bShowingSlider;
};

#endif
