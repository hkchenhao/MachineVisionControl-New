#include "ButtonSelForm.h"
#include "ui_ButtonSelForm.h"
#include "Event/MainFormEvent.h"
#include "Form/MainForm.h"
#include "Mgr/FormFrame.h"
#include "Utils/ButtonInfo.h"
#include "Utils/QJsonAnalysis.h"
#include "Utils/SystemUtils.h"
#include <QDebug>

const QString IMAGE_TYPE(".jpg");       // 纽扣图像的格式类型
const qint32 PER_ROW_COUNT = 5;         // 每页显示的行数
const qint32 PER_COLUMN_COUNT = 4;      // 每页显示的列数

/********** 纽扣配置Json信息类 **********/
ButtonJsonInfo::ButtonJsonInfo(const QString& filepath, const QString& filename, QWidget* parent) : QWidget(parent)
{
    // 初始化配置文件所在路径与名称
    p_configFilePath = new QString(filepath);
    p_configFileName = new QString(filename);
    // 读取纽扣配置参数json文件
    p_buttonInfo = new QJsonAnalysis(*p_configFilePath + "/" + *p_configFileName + ".ini", true);
    // 初始化p_buttonImage、p_buttonName与p_layout
    p_layout = new QVBoxLayout;
    p_buttonImage = new QLabel;
    p_buttonName = new QLabel(p_configFileName->section('.', 0, 0));
    p_buttonImage->installEventFilter(this);
    p_buttonName->installEventFilter(this);
    // 配置label布局与属性
    p_buttonImage->setFixedSize(96, 96);
    p_buttonImage->setAlignment(Qt::AlignCenter);
    // 判断纽扣图片是否存在
    QString imagepath(*p_configFilePath + "/" + *p_configFileName + IMAGE_TYPE);
    QFileInfo imagefileinfo(imagepath);
    if(imagefileinfo.isFile())
    {
        QImage buttonimage(imagepath);
        QImage buttonimagescaled = buttonimage.scaled(p_buttonImage->width(), p_buttonImage->height());
        p_buttonImage->setPixmap(QPixmap::fromImage(buttonimagescaled));
    }
    else
    {
        QImage buttonimage(":/pic/button_128px.png");
        QImage buttonimagescaled = buttonimage.scaled(p_buttonImage->width(), p_buttonImage->height());
        p_buttonImage->setPixmap(QPixmap::fromImage(buttonimagescaled));
    }
    // 配置p_buttonName
    QFont font;
    font.setPointSize(12);
    font.setBold(false);
    font.setWeight(16);
    p_buttonName->setFont(font);
    p_buttonName->setFixedSize(96, 16);
    p_buttonName->setAlignment(Qt::AlignCenter);
    // 配置Label布局
    p_layout->setSpacing(5);
    p_layout->addWidget(p_buttonImage);
    p_layout->addWidget(p_buttonName);
    // label加入到父对象
    this->setLayout(p_layout);
}

ButtonJsonInfo::~ButtonJsonInfo()
{
    if(p_configFilePath) { delete p_configFilePath; }
    if(p_configFileName) { delete p_configFileName; }
    if(p_buttonInfo) { delete p_buttonInfo; }
}

// [虚函数覆盖]事件过滤器函数
bool ButtonJsonInfo::eventFilter(QObject* watched, QEvent* event)
{
    // 处理buttonImage-Label的鼠标单击事件
    if(watched == p_buttonImage)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            emit SignalCmd_ButtonImageSelected(this);
            return true;
        }
        else
            return false;
    }
    else
        return QWidget::eventFilter(watched, event);
}

/*========================================================================================*/
/********** 纽扣选择界面类 **********/
ButtonSelForm::ButtonSelForm(QWidget* parent) : QWidget(parent), ui(new Ui::ButtonSelWidget)
{
    ui->setupUi(this);
    InitFormWidget();
    LoadAllConfigFileInfo();
    ShowButtonImage();
}

ButtonSelForm::~ButtonSelForm()
{
    delete ui;
    foreach (ButtonJsonInfo* p_buttoninfo, v_pButtonOriginalInfo)
        delete p_buttoninfo;
}

// [成员函数]初始化界面widget
void ButtonSelForm::InitFormWidget()
{
    QStringList buttominfo_cz("无");
    for(QString& str : ButtonMaterialStrCnBuf) { buttominfo_cz.append(str); }
    ui->comboBox_CZ->addItems(buttominfo_cz);
    QStringList buttominfo_xz("无");
    for(QString& str : ButtonShapeStrCnBuf) { buttominfo_xz.append(str); }
    ui->comboBox_XZ->addItems(buttominfo_xz);
    QStringList buttominfo_cc("无");
    for(QString& str : ButtonSizeBuf) { buttominfo_cc.append(str); }
    ui->comboBox_CC->addItems(buttominfo_cc);
    QStringList buttominfo_xks("无");
    for(QString& str : ButtonHoleNumStrCnBuf) { buttominfo_xks.append(str); }
    ui->comboBox_XKS->addItems(buttominfo_xks);
    QStringList buttominfo_tmx("无");
    for(QString& str : ButtonLightStrCnBuf) { buttominfo_tmx.append(str); }
    ui->comboBox_TMX->addItems(buttominfo_tmx);
    QStringList buttominfo_hs("无");
    for(QString& str : ButtonPatternStrCnBuf) { buttominfo_hs.append(str); }
    ui->comboBox_HS->addItems(buttominfo_hs);
    QStringList buttominfo_zs("无");
    for(QString& str : ButtonColorStrCnBuf) { buttominfo_zs.append(str); }
    ui->comboBox_ZS->addItems(buttominfo_zs);
}

// [成员函数]加载所有ini配置文件信息
void ButtonSelForm::LoadAllConfigFileInfo()
{
    // 列出配置文件目录下所有的ini纽扣配置文件
    //QString inifilepath_name(QCoreApplication::applicationDirPath() + QString("/Configs/"));
//    QDir inifilepath(SystemUtils::GetPathForButtonConfigFile(), QString("*.ini"));
//    QStringList inifilename_list = inifilepath.entryList(QDir::Files, QDir::Name);
//    qint32 inifilecount = inifilename_list.size();
    //v_pButtonOriginalInfo.resize(inifilecount);

    QDir path(SystemUtils::GetPathForButtonConfigFile());
    QStringList buttonnamelist(path.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name));
    qint32 dircount = buttonnamelist.size();

    // 生成全部纽扣信息widget指针vector
    p_currentButtonInfoVector = &v_pButtonOriginalInfo;
    p_currentButtonPageInfoStruct = &buttonOriginalPageInfo;
    for(qint32 i = 0; i < dircount; i++)
    {
        QString pathname(SystemUtils::GetPathForButtonConfigFile() + buttonnamelist.at(i));
        QString filename(buttonnamelist.at(i));
        p_currentButtonInfoVector->push_back(new ButtonJsonInfo(pathname, filename));
        QObject::connect(p_currentButtonInfoVector->at(i), &ButtonJsonInfo::SignalCmd_ButtonImageSelected,
                         this, &ButtonSelForm::SetButtonSelectStaus);
    }
    // 初始化上一个以及当前被选取的纽扣图像指针
    p_lastSeletedButton = nullptr;
    p_currentSeletedButton = nullptr;
    // 初始化与翻页有关的变量
    InitButtonPageInfo();
}

// [成员函数]显示纽扣图片
void ButtonSelForm::ShowButtonImage()
{
    // 局部变量
    qint32 onepagecount = PER_ROW_COUNT * PER_COLUMN_COUNT;
    qint32 i = 0, j = 0;
    // 显示页码信息
    ui->labelPageAll->setText(QString("共%1页").arg(p_currentButtonPageInfoStruct->buttonImagePageCount));
    ui->labelPageCurrent->setText(QString("第%1页").arg(p_currentButtonPageInfoStruct->buttonImageCurrentPageNum));
    // 清除当前界面的纽扣图像
    while(ui->ButtonImageGridLayout->count() > 0)
    {
        QWidget* currentwidget = ui->ButtonImageGridLayout->itemAt(0)->widget();
        ui->ButtonImageGridLayout->removeWidget(currentwidget);
        // 如果currentwidget不是ButtonInfo就释放掉
        if(qobject_cast<ButtonJsonInfo*>(currentwidget) == 0)
            delete currentwidget;
        else
            currentwidget->setParent(nullptr);
    }
    // 显示新的一页图像
    if(p_currentButtonPageInfoStruct->buttonImageCurrentPageNum < p_currentButtonPageInfoStruct->buttonImagePageCount)
    {
        j = 0;
        for(i = ((p_currentButtonPageInfoStruct->buttonImageCurrentPageNum - 1) * onepagecount);\
                  i < p_currentButtonPageInfoStruct->buttonImageCurrentPageNum * onepagecount; i++, j++)
        {
            ui->ButtonImageGridLayout->addWidget(p_currentButtonInfoVector->at(i), j / PER_COLUMN_COUNT, j % PER_COLUMN_COUNT, Qt::AlignCenter);
        }
    }
    else
    {
        j = 0;
        for(i = ((p_currentButtonPageInfoStruct->buttonImageCurrentPageNum - 1) * onepagecount);\
                  i < p_currentButtonInfoVector->size(); i++, j++)
        {
            ui->ButtonImageGridLayout->addWidget(p_currentButtonInfoVector->at(i), j / PER_COLUMN_COUNT, j % PER_COLUMN_COUNT, Qt::AlignCenter);
        }
        for(i = p_currentButtonPageInfoStruct->buttonImageLastPageCount; i < onepagecount; i++, j++)
        {
            ui->ButtonImageGridLayout->addWidget(new QWidget, j / PER_COLUMN_COUNT, j % PER_COLUMN_COUNT, Qt::AlignCenter);
        }
    }
}

// [成员函数]计算纽扣页面变量
void ButtonSelForm::InitButtonPageInfo()
{
    qint32 buttoninfowidget_count = p_currentButtonInfoVector->size();
    qint32 onepagecount = PER_ROW_COUNT * PER_COLUMN_COUNT;
    p_currentButtonPageInfoStruct->buttonImageCurrentPageNum = 1;
    if((buttoninfowidget_count % onepagecount) == 0)
    {
        p_currentButtonPageInfoStruct->buttonImagePageCount = buttoninfowidget_count / onepagecount;
        p_currentButtonPageInfoStruct->buttonImageLastPageCount = onepagecount;
    }
    else
    {
        p_currentButtonPageInfoStruct->buttonImagePageCount = buttoninfowidget_count / onepagecount + 1;
        p_currentButtonPageInfoStruct->buttonImageLastPageCount = buttoninfowidget_count % onepagecount;
    }
    // 上下翻页按键处理
    ui->pushButton_LastPage->setEnabled(false);
    if(p_currentButtonPageInfoStruct->buttonImagePageCount == 0)
        ui->pushButton_NextPage->setEnabled(false);
    else
        ui->pushButton_NextPage->setEnabled(true);
}

// [成员函数]根据条件刷新纽扣信息区信息
void ButtonSelForm::UpdateButtonInfoLabel(bool isshow)
{
    if(isshow)
    {
        QString jsonparentstr;
        QMap<QString, qint32>::iterator it;
        // 纽扣图片/编号/时间信息
        ui->ButtonImage->setPixmap(*p_currentSeletedButton->GetButtonImagePtr()->pixmap());
        ui->LabelInfo_BH->setText(p_currentSeletedButton->GetButtonInfoPtr()->getString("name"));
        ui->LabelInfo_SJ->setText(p_currentSeletedButton->GetButtonInfoPtr()->getString("time"));
        // 纽扣基本信息
        jsonparentstr = QString("infoFront.");
        it = ButtonMaterialMap.find(p_currentSeletedButton->GetButtonInfoPtr()->getString(jsonparentstr + "materialF"));
        if(it != ButtonMaterialMap.end())
            ui->LabelInfo_CZ->setText(ButtonMaterialStrCnBuf[it.value()]);
        it = ButtonShapeMap.find(p_currentSeletedButton->GetButtonInfoPtr()->getString(jsonparentstr + "shapeF"));
        if(it != ButtonShapeMap.end())
            ui->LabelInfo_XZ->setText(ButtonShapeStrCnBuf[it.value()]);
        it = ButtonHoleNumMap.find(p_currentSeletedButton->GetButtonInfoPtr()->getString(jsonparentstr + "holeNumF"));
        if(it != ButtonHoleNumMap.end())
            ui->LabelInfo_XKS->setText(ButtonHoleNumStrCnBuf[it.value()]);
        it = ButtonLightMap.find(p_currentSeletedButton->GetButtonInfoPtr()->getString(jsonparentstr + "lightF"));
        if(it != ButtonLightMap.end())
            ui->LabelInfo_TMX->setText(ButtonLightStrCnBuf[it.value()]);
        it = ButtonPatternMap.find(p_currentSeletedButton->GetButtonInfoPtr()->getString(jsonparentstr + "patternF"));
        if(it != ButtonPatternMap.end())
            ui->LabelInfo_HS->setText(ButtonPatternStrCnBuf[it.value()]);
        it = ButtonColorMap.find(p_currentSeletedButton->GetButtonInfoPtr()->getString(jsonparentstr + "colorF"));
        if(it != ButtonColorMap.end())
            ui->LabelInfo_ZS->setText(ButtonColorStrCnBuf[it.value()]);
        ui->LabelInfo_CC->setText(p_currentSeletedButton->GetButtonInfoPtr()->getString(jsonparentstr + "sizeF"));
        // 纽扣尺寸信息
        jsonparentstr = QString("taskSize.");
        double buttonsize, buttonsizeup, buttonsizedown;
        QString buttonsizestr;
        buttonsize = p_currentSeletedButton->GetButtonInfoPtr()->getDouble(jsonparentstr + "outDia");
        buttonsizeup = p_currentSeletedButton->GetButtonInfoPtr()->getDouble(jsonparentstr + "outDiaDevUp");
        buttonsizedown = p_currentSeletedButton->GetButtonInfoPtr()->getDouble(jsonparentstr + "outDiaDevDown");
        buttonsizestr = QString("%1[%2,%3]").arg(QString::number(buttonsize,'f',1)).arg(buttonsizeup).arg(buttonsizedown);
        ui->LabelInfo_WJ->setText(buttonsizestr);
        buttonsize = p_currentSeletedButton->GetButtonInfoPtr()->getDouble(jsonparentstr + "holeDia");
        buttonsizeup = p_currentSeletedButton->GetButtonInfoPtr()->getDouble(jsonparentstr + "holeDiaDevUp");
        buttonsizedown = p_currentSeletedButton->GetButtonInfoPtr()->getDouble(jsonparentstr + "holeDiaDevDown");
        buttonsizestr = QString("%1[%2,%3]").arg(QString::number(buttonsize,'f',1)).arg(buttonsizeup).arg(buttonsizedown);
        ui->LabelInfo_XKJ->setText(buttonsizestr);
        buttonsize = p_currentSeletedButton->GetButtonInfoPtr()->getDouble(jsonparentstr + "holeDist");
        buttonsizeup = p_currentSeletedButton->GetButtonInfoPtr()->getDouble(jsonparentstr + "holeDistDevUp");
        buttonsizedown = p_currentSeletedButton->GetButtonInfoPtr()->getDouble(jsonparentstr + "holeDistDevDown");
        buttonsizestr = QString("%1[%2,%3]").arg(QString::number(buttonsize,'f',1)).arg(buttonsizeup).arg(buttonsizedown);
        ui->LabelInfo_XKJL->setText(buttonsizestr);

    }
    else
    {
        ui->LabelInfo_BH->setText("无");
        ui->LabelInfo_SJ->setText("无");

        ui->LabelInfo_CZ->setText("无");
        ui->LabelInfo_XZ->setText("无");
        ui->LabelInfo_XKS->setText("0");
        ui->LabelInfo_TMX->setText("无");
        ui->LabelInfo_HS->setText("无");
        ui->LabelInfo_ZS->setText("无");
        ui->LabelInfo_CC->setText("0.0");

        ui->LabelInfo_WJ->setText("0.0[0.0,0.0]");
        ui->LabelInfo_XKJ->setText("0.0[0.0,0.0]");
        ui->LabelInfo_XKJL->setText("0.0[0.0,0.0]");
    }
}

// [slot函数]选中某个图片后触发
void ButtonSelForm::SetButtonSelectStaus(ButtonJsonInfo* p_buttoninfo)
{
    // 添加选中边框
    p_currentSeletedButton = p_buttoninfo;
    if(p_lastSeletedButton != p_currentSeletedButton)
    {
        if(p_lastSeletedButton != nullptr)
            p_lastSeletedButton->GetButtonImagePtr()->setStyleSheet("");
        p_currentSeletedButton->GetButtonImagePtr()->setStyleSheet("border:4px solid #FF0000");
        // 记录选中的纽扣信息文件名称
        currentSeletedButtonName = p_currentSeletedButton->GetButtonNamePtr()->text();
        p_lastSeletedButton = p_currentSeletedButton;
    }
    // 更新信息区信息
    UpdateButtonInfoLabel(true);
}

// [控件slot函数]纽扣图像上一页按键
void ButtonSelForm::on_pushButton_LastPage_clicked()
{
    // 按键锁定判断
    if(p_currentButtonPageInfoStruct->buttonImageCurrentPageNum  == p_currentButtonPageInfoStruct->buttonImagePageCount)
        ui->pushButton_NextPage->setEnabled(true);
    if((p_currentButtonPageInfoStruct->buttonImageCurrentPageNum -= 1) == 1)
        ui->pushButton_LastPage->setEnabled(false);
    // 刷新图像显示
    ShowButtonImage();
}

// [控件slot函数]纽扣图像下一页按键
void ButtonSelForm::on_pushButton_NextPage_clicked()
{
    // 按键锁定判断
    if(p_currentButtonPageInfoStruct->buttonImageCurrentPageNum  == 1)
        ui->pushButton_LastPage->setEnabled(true);
    if((p_currentButtonPageInfoStruct->buttonImageCurrentPageNum += 1) == p_currentButtonPageInfoStruct->buttonImagePageCount)
        ui->pushButton_NextPage->setEnabled(false);
    // 刷新图像显示
    ShowButtonImage();
}

// [控件slot函数]纽扣取消选择按键
void ButtonSelForm::on_pushButton_Esc_clicked()
{
    // 清除当前Form
    FormFrame::GetInstance()->p_formstacked_->removeWidget(FormFrame::GetInstance()->p_buttonselform_);
    delete FormFrame::GetInstance()->p_buttonselform_;
    FormFrame::GetInstance()->p_buttonselform_ = nullptr;
    FormFrame::GetInstance()->formstacked_id_.buttonselform_id = -1;
    FormFrame::GetInstance()->p_formstacked_->setCurrentIndex(FormFrame::GetInstance()->formstacked_id_.mainform_id);
}

// [控件slot函数]纽扣确定选择按键
void ButtonSelForm::on_pushButton_Enter_clicked()
{
    // 向MainForm投递一个事件（被选纽扣的ID编号）
    MainFormEvent* mainformevent = new MainFormEvent(MainFormEvent::EventType_ButtonSelectedResult);
    mainformevent->button_id_ = currentSeletedButtonName;
    // 注意一定要加入mainform的头文件，否则post出错
    QCoreApplication::postEvent(FormFrame::GetInstance()->p_mainform_, mainformevent);
    // 清除当前Form
    FormFrame::GetInstance()->p_formstacked_->removeWidget(FormFrame::GetInstance()->p_buttonselform_);
    delete FormFrame::GetInstance()->p_buttonselform_;
    FormFrame::GetInstance()->p_buttonselform_ = nullptr;
    FormFrame::GetInstance()->formstacked_id_.buttonselform_id = -1;
    FormFrame::GetInstance()->p_formstacked_->setCurrentIndex(FormFrame::GetInstance()->formstacked_id_.mainform_id);
}

// [控件slot函数]纽扣检索按键
void ButtonSelForm::on_pushButton_Find_clicked()
{
    // 指针指向筛选纽扣图像缓存区
    p_currentButtonInfoVector = &v_pButtonSelectedInfo;
    p_currentButtonInfoVector->resize(0);
    p_currentButtonPageInfoStruct = &buttonSelectedPageInfo;
    // 首先根据纽扣ID号查询
    QString buttonid = ui->lineEdit_ButtonID->text();
    foreach (ButtonJsonInfo* p_buttoninfo, v_pButtonOriginalInfo)
    {
        if(p_buttoninfo->GetButtonInfoPtr()->getString("name").contains(buttonid, Qt::CaseInsensitive))
            p_currentButtonInfoVector->push_back(p_buttoninfo);
    }
    // 显示查询结果
    InitButtonPageInfo();
    ShowButtonImage();
}

// [控件slot函数]纽扣清空检索条件按键
void ButtonSelForm::on_pushButton_Clear_clicked()
{
    // 指针指向原始纽扣图像缓存区
    p_currentButtonInfoVector = &v_pButtonOriginalInfo;
    p_currentButtonPageInfoStruct = &buttonOriginalPageInfo;
    p_currentButtonPageInfoStruct->buttonImageCurrentPageNum = 1;
    // 复位检索条件
    ui->lineEdit_ButtonID->setText("");
    // 复位已经选中的图片
    if(p_currentSeletedButton)
    {
        p_currentSeletedButton->GetButtonImagePtr()->setStyleSheet("");
        UpdateButtonInfoLabel(false);
        currentSeletedButtonName.clear();
        p_currentSeletedButton = nullptr;
    }
    // 刷新图像显示
    ShowButtonImage();
}


