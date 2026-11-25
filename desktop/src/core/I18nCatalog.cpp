#include "I18nCatalog.h"

#include <QVariantMap>

namespace FireSystem {

namespace {

int languageIndex(const QString &lang)
{
    if (lang == QLatin1String("en")) return 0;
    if (lang == QLatin1String("ru")) return 1;
    if (lang == QLatin1String("de")) return 2;
    if (lang == QLatin1String("fr")) return 3;
    if (lang == QLatin1String("es")) return 4;
    return -1; // zh-CN
}

} // namespace

I18nCatalog::I18nCatalog(QObject *parent)
    : QObject(parent)
{
    auto add = [this](const char *zh, const char *en, const char *ru, const char *de, const char *fr, const char *es) {
        m_dict.insert(QString::fromUtf8(zh), {
            QString::fromUtf8(en),
            QString::fromUtf8(ru),
            QString::fromUtf8(de),
            QString::fromUtf8(fr),
            QString::fromUtf8(es)
        });
    };

    // Shell, Navigation & Profile Control
    add("轨道消防上位机", "Rail fire protection console", "Пульт пожарной защиты поездов", "Bahnbrandschutz-Leitstand", "Console incendie ferroviaire", "Consola de incendios ferroviarios");
    add("设置", "Settings", "Настройки", "Einstellungen", "Paramètres", "Configuración");
    add("账号", "Account", "Учётная запись", "Konto", "Compte", "Cuenta");
    add("账号设置", "Account settings", "Настройки учётных записей", "Kontoeinstellungen", "Paramètres de compte", "Configuración de la cuenta");
    add("许可证", "License", "Лицензия", "Lizenz", "Licence", "Licencia");
    add("项目模板", "Project template", "Шаблон проекта", "Projektvorlage", "Modèle de projet", "Plantilla de proyecto");
    add("通用", "General", "Общие", "Allgemein", "Général", "General");
    add("关于", "About", "О программе", "Info", "À propos", "Acerca de");
    add("登录", "Sign in", "Войти", "Anmelden", "Se connecter", "Iniciar sesión");
    add("切换账号", "Switch account", "Сменить учётную запись", "Konto wechseln", "Changer de compte", "Cambiar cuenta");
    add("退出登录", "Sign Out", "Выйти", "Abmelden", "Déconnexion", "Cerrar sesión");
    add("本地仿真", "Local simulation", "Локальная модель", "Lokale Simulation", "Simulation locale", "Simulación local");
    add("管理工作站外观、账号、模板与版本。", "Manage appearance, accounts, templates and versions.", "Настраивайте оформление, учётные записи, шаблоны и версии.", "Darstellung, Konten, Vorlagen und Versionen verwalten.", "Gérez l’apparence, les comptes, les modèles et les versions.", "Gestione la apariencia, las cuentas, las plantillas y las versiones.");
    add("管理模拟工作站的身份与离线许可。", "Manage workstation sign-in and the simulated offline license.", "Вход на станцию и автономная демонстрационная лицензия.", "Anmeldung und simulierte Offline-Lizenz der Station verwalten.", "Gérez la session et la licence hors ligne de la station simulée.", "Gestione el acceso y la licencia sin conexión de la estación simulada.");

    add("运行监控", "Operations", "Мониторинг", "Betriebsüberwachung", "Supervision", "Supervisión");
    add("运行总览", "Overview", "Обзор", "Übersicht", "Vue d’ensemble", "Vista general");
    add("设备监控", "Device monitor", "Мониторинг устройств", "Geräteüberwachung", "Suivi des équipements", "Monitor de equipos");
    add("告警中心", "Alarms", "Тревоги", "Alarme", "Alarmes", "Alarmas");
    add("工程配置", "Engineering", "Проектирование", "Projektierung", "Ingénierie", "Ingeniería");
    add("参数配置", "Parameters", "Параметры", "Parameter", "Paramètres", "Parámetros");
    add("协议设计", "Protocol designer", "Редактор протоколов", "Protokolleditor", "Éditeur de protocoles", "Editor de protocolos");
    add("流程设计", "Workflow designer", "Редактор процессов", "Ablaufeditor", "Éditeur de séquences", "Editor de secuencias");
    add("维护工具", "Maintenance", "Обслуживание", "Wartung", "Maintenance", "Mantenimiento");
    add("控制与气瓶", "Control & cylinders", "Управление и баллоны", "Steuerung und Flaschen", "Commande et bouteilles", "Control y botellas");
    add("固件升级", "Firmware Update", "Обновление прошивки", "Firmware-Update", "Mise à jour du micrologiciel", "Actualización de firmware");
    add("日志与审计", "Logs & audit", "Журналы и аудит", "Protokolle und Audit", "Journaux et audit", "Registros y auditoría");
    add("终端工具", "Terminals", "Терминалы", "Terminals", "Terminaux", "Terminales");
    add("帮助", "Help", "Справка", "Hilfe", "Aide", "Ayuda");
    add("项目选择", "Projects", "Выбор проекта", "Projektauswahl", "Choix du projet", "Selección de proyecto");
    add("工作站", "Workstation", "Рабочая станция", "Arbeitsstation", "Station de travail", "Estación de trabajo");
    add("切换导航", "Toggle navigation", "Переключить навигацию", "Navigation umschalten", "Basculer la navigation", "Alternar navegación");
    add("环线基准项目", "Ring line baseline", "Базовый проект кольцевой линии", "Ringlinie – Basismodell", "Projet de base de ligne circulaire", "Proyecto base de línea circular");
    add("评审记录", "Review notes", "Журнал замечаний", "Prüfanmerkungen", "Notes de revue", "Notas de revisión");
    add("页面评审", "Page review", "Оценка страницы", "Seitenprüfung", "Revue de la page", "Revisión de página");
    add("页面搜索", "Find a page", "Поиск страницы", "Seitensuche", "Recherche de page", "Buscar página");
    add("搜索页面", "Find a page", "Поиск страницы", "Seite suchen", "Rechercher une page", "Buscar página");
    add("输入页面名称", "Enter a page name", "Введите название страницы", "Seitennamen eingeben", "Saisissez le nom d’une page", "Introduzca el nombre de una página");
    add("未找到页面", "Page not found", "Страница не найдена", "Seite nicht gefunden", "Page introuvable", "Página no encontrada");

    // Roles & Badges
    add("操作员", "Operator", "Оператор", "Bediener", "Opérateur", "Operador");
    add("管理员", "Administrator", "Администратор", "Administrator", "Administrateur", "Administrador");
    add("调试员", "Debugger", "Наладчик", "Inbetriebnehmer", "Technicien de mise au point", "Técnico de depuración");
    add("只读", "Read-only", "Только чтение", "Schreibgeschützt", "Lecture seule", "Solo lectura");
    add("启用", "Enabled", "Включено", "Aktiviert", "Activé", "Activado");
    add("停用", "Disabled", "Отключено", "Deaktiviert", "Désactivé", "Desactivado");
    add("待设置密码", "Password required", "Требуется пароль", "Passwort erforderlich", "Mot de passe requis", "Contraseña pendiente");
    add("有效", "Valid", "Действительно", "Gültig", "Valide", "Válido");
    add("已失效", "Expired", "Истекла", "Abgelaufen", "Expirée", "Caducada");
    add("待复核", "Pending review", "Ожидает проверки", "Prüfung ausstehend", "À vérifier", "Pendiente de revisión");
    add("已复核", "Reviewed", "Проверено", "Geprüft", "Vérifié", "Revisado");
    add("跟随系统", "Use system setting", "Как в системе", "Systemeinstellung verwenden", "Suivre le système", "Usar la configuración del sistema");
    add("浅色", "Light", "Светлая", "Hell", "Clair", "Claro");
    add("深色", "Dark", "Тёмная", "Dunkel", "Sombre", "Oscuro");
    add("语言", "Language", "Язык", "Sprache", "Langue", "Idioma");
    add("界面主题", "Theme", "Тема", "Design", "Thème", "Tema");

    // Settings Import / Export
    add("导入设置", "Import settings", "Импортировать настройки", "Einstellungen importieren", "Importer les paramètres", "Importar configuración");
    add("导出设置", "Export settings", "Экспортировать настройки", "Einstellungen exportieren", "Exporter les paramètres", "Exportar configuración");
    add("确认导入设置", "Confirm settings import", "Подтвердить импорт настроек", "Import der Einstellungen bestätigen", "Confirmer l’importation des paramètres", "Confirmar la importación de la configuración");
    add("将应用文件中的语言和主题。", "The language and theme from the file will be applied.", "Будут применены язык и тема из файла.", "Sprache und Design aus der Datei werden übernommen.", "La langue et le thème du fichier seront appliqués.", "Se aplicarán el idioma y el tema del archivo.");
    add("将用文件中的 {0} 个账号替换本机的操作员和管理员账号，并退出登录。", "The {0} accounts in the file will replace local operators and administrators, and you will be signed out.", "{0} учётных записей из файла заменят местные записи операторов и администраторов. Сеанс завершится.", "Die {0} Konten aus der Datei ersetzen die lokalen Bediener- und Administratorkonten. Sie werden abgemeldet.", "Les {0} comptes du fichier remplaceront les comptes opérateurs et administrateurs locaux. Votre session sera fermée.", "Las {0} cuentas del archivo sustituirán a las cuentas locales de operador y administrador, y se cerrará la sesión.");
    add("确认导入", "Confirm import", "Подтвердить импорт", "Import bestätigen", "Confirmer l’importation", "Confirmar importación");
    add("取消", "Cancel", "Отмена", "Abbrechen", "Annuler", "Cancelar");
    add("关闭", "Close", "Закрыть", "Schließen", "Fermer", "Cerrar");
    add("请选择有效的设置文件。", "Select a valid settings file.", "Выберите действительный файл настроек.", "Wählen Sie eine gültige Einstellungsdatei aus.", "Sélectionnez un fichier de paramètres valide.", "Seleccione un archivo de configuración válido.");
    add("设置已导入", "Settings imported", "Настройки импортированы", "Einstellungen importiert", "Paramètres importés", "Configuración importada");
    add("无法保存通用设置。", "Could not save general settings.", "Не удалось сохранить общие настройки.", "Allgemeine Einstellungen konnten nicht gespeichert werden.", "Impossible d’enregistrer les paramètres généraux.", "No se pudo guardar la configuración general.");
    add("账号已导入，但通用设置保存失败。", "Accounts imported. General settings could not be saved.", "Учётные записи импортированы. Общие настройки не сохранены.", "Konten importiert. Allgemeine Einstellungen konnten nicht gespeichert werden.", "Comptes importés. Les paramètres généraux n’ont pas pu être enregistrés.", "Cuentas importadas. No se pudo guardar la configuración general.");

    // Accounts & Login
    add("本机账号", "Local account", "Локальная учётная запись", "Lokales Konto", "Compte local", "Cuenta local");
    add("新增账号", "Add account", "Создать аккаунт", "Konto hinzufügen", "Ajouter un compte", "Añadir cuenta");
    add("搜索本机账号", "Find local account", "Найти локальный аккаунт", "Lokales Konto suchen", "Rechercher un compte local", "Buscar cuenta local");
    add("搜索账号或角色", "Search by account or role", "Поиск по аккаунту или роли", "Konto oder Rolle suchen", "Rechercher un compte ou un rôle", "Buscar cuenta o rol");
    add("角色", "Role", "Роль", "Rolle", "Rôle", "Rol");
    add("状态", "Status", "Состояние", "Status", "État", "Estado");
    add("操作", "Action", "Действие", "Aktion", "Action", "Acción");
    add("编辑", "Edit", "Изменить", "Bearbeiten", "Modifier", "Editar");
    add("删除", "Delete", "Удалить", "Löschen", "Supprimer", "Eliminar");
    add("没有匹配的账号。", "No matching accounts.", "Нет подходящих учётных записей.", "Keine passenden Konten.", "Aucun compte correspondant.", "No hay cuentas coincidentes.");
    add("新增本机账号", "New local account", "Новый локальный аккаунт", "Neues lokales Konto", "Nouveau compte local", "Nueva cuenta local");
    add("编辑本机账号", "Edit local account", "Изменить локальный аккаунт", "Lokales Konto bearbeiten", "Modifier le compte local", "Editar cuenta local");
    add("密码", "Password", "Пароль", "Passwort", "Mot de passe", "Contraseña");
    add("密码（可留空）", "Password (optional)", "Пароль (необязательно)", "Passwort (optional)", "Mot de passe (facultatif)", "Contraseña (opcional)");
    add("设置密码", "Set password", "Задать пароль", "Passwort festlegen", "Définir le mot de passe", "Establecer contraseña");
    add("重设密码", "Reset password", "Сбросить пароль", "Passwort zurücksetzen", "Réinitialiser le mot de passe", "Restablecer contraseña");
    add("启用此账号", "Enable account", "Включить аккаунт", "Konto aktivieren", "Activer ce compte", "Activar cuenta");
    add("保存账号", "Save account", "Сохранить аккаунт", "Konto speichern", "Enregistrer le compte", "Guardar cuenta");
    add("当前账号", "Current account", "Текущая учётная запись", "Aktuelles Konto", "Compte actuel", "Cuenta actual");
    add("修改自己的密码", "Change my password", "Изменить свой пароль", "Eigenes Passwort ändern", "Modifier mon mot de passe", "Cambiar mi contraseña");
    add("当前密码", "Current password", "Текущий пароль", "Aktuelles Passwort", "Mot de passe actuel", "Contraseña actual");
    add("新密码", "New password", "Новый пароль", "Neues Passwort", "Nouveau mot de passe", "Nueva contraseña");
    add("确认新密码", "Confirm new password", "Подтвердить новый пароль", "Neues Passwort bestätigen", "Confirmer le nouveau mot de passe", "Confirmar nueva contraseña");
    add("保存新密码", "Save new password", "Сохранить новый пароль", "Neues Passwort speichern", "Enregistrer le nouveau mot de passe", "Guardar nueva contraseña");
    add("删除账号", "Delete account", "Удалить учётную запись", "Konto löschen", "Supprimer le compte", "Eliminar cuenta");
    add("确认删除", "Confirm deletion", "Подтвердить удаление", "Löschen bestätigen", "Confirmer la suppression", "Confirmar eliminación");
    add("首次使用 admin 登录时，密码留空。", "Leave the password blank the first time you sign in as admin.", "При первом входе под учётной записью admin оставьте пароль пустым.", "Lassen Sie bei der ersten Anmeldung als admin das Passwort leer.", "À la première connexion avec le compte admin, laissez le mot de passe vide.", "Deje la contraseña en blanco al iniciar sesión como admin por primera vez.");
    add("账号或密码错误", "Incorrect account or password", "Неверная учётная запись или пароль", "Konto oder Passwort falsch", "Compte ou mot de passe incorrect", "Cuenta o contraseña incorrecta");
    add("两次输入的密码不一致", "Passwords do not match", "Пароли не совпадают", "Passwörter stimmen nicht überein", "Les mots de passe ne correspondent pas", "Las contraseñas no coinciden");
    add("密码已更新", "Password updated", "Пароль обновлён", "Passwort aktualisiert", "Mot de passe mis à jour", "Contraseña actualizada");
    add("当前密码错误", "Incorrect current password", "Неверный текущий пароль", "Aktuelles Passwort falsch", "Mot de passe actuel incorrect", "Contraseña actual incorrecta");
    add("请输入密码", "Enter a password", "Введите пароль", "Passwort eingeben", "Saisissez un mot de passe", "Introduzca una contraseña");
    add("此账号已存在", "This account already exists.", "Эта учётная запись уже существует.", "Dieses Konto ist bereits vorhanden.", "Ce compte existe déjà.", "La cuenta ya existe.");
    add("请检查账号和角色", "Check the account and role.", "Проверьте учётную запись и роль.", "Konto und Rolle prüfen.", "Vérifiez le compte et le rôle.", "Compruebe la cuenta y el rol.");
    add("需要账号管理权限", "Account management permission required.", "Требуются права управления учётными записями.", "Kontoverwaltungsrechte erforderlich.", "Droits de gestion des comptes requis.", "Se requieren permisos de gestión de cuentas.");
    add("账号库无法读取", "Unable to read the account database.", "Не удалось прочитать базу учётных записей.", "Kontendatenbank kann nicht gelesen werden.", "Impossible de lire la base des comptes.", "No se puede leer la base de cuentas.");
    add("账号已变更，请重新登录", "Account changed. Please sign in again.", "Учётная запись изменена. Войдите снова.", "Konto geändert. Bitte erneut anmelden.", "Le compte a changé. Veuillez vous reconnecter.", "La cuenta ha cambiado. Vuelva a iniciar sesión.");
    add("此账号已变更", "This account has changed.", "Эта учётная запись изменилась.", "Dieses Konto wurde geändert.", "Ce compte a été modifié.", "Esta cuenta ha cambiado.");
    add("请在登录页修改自己的密码", "Change your own password or keep your role active.", "Измените свой пароль или сохраните свою роль активной.", "Ändern Sie Ihr eigenes Passwort, ohne Ihre Rolle zu deaktivieren.", "Modifiez votre propre mot de passe sans désactiver votre rôle.", "Cambie su propia contraseña sin desactivar su rol.");
    add("请为账号设置密码", "Set a password for the account.", "Задайте пароль для учётной записи.", "Legen Sie ein Passwort für das Konto fest.", "Définissez un mot de passe pour le compte.", "Establezca una contraseña para la cuenta.");
    add("账号备份无效或含受保护账号", "Account backup is invalid or contains protected accounts.", "Резервная копия некорректна или содержит защищённые записи.", "Kontosicherung ungültig oder enthält geschützte Konten.", "La sauvegarde des comptes est invalide ou contient des comptes protégés.", "La copia de cuentas no es válida o contiene cuentas protegidas.");
    add("此账号无法删除", "This account cannot be deleted.", "Эту учётную запись нельзя удалить.", "Dieses Konto kann nicht gelöscht werden.", "Ce compte ne peut pas être supprimé.", "Esta cuenta no se puede eliminar.");
    add("恢复请求已失效", "Recovery request has expired.", "Запрос на восстановление недействителен.", "Wiederherstellungsanforderung ist abgelaufen.", "La demande de récupération a expiré.", "La solicitud de recuperación ha caducado.");
    add("没有可解锁的管理员账号", "No disabled administrator account to unlock.", "Нет заблокированных учётных записей администратора.", "Kein gesperrtes Administratorkonto zum Entsperren vorhanden.", "Aucun compte administrateur désactivé à déverrouiller.", "No hay ninguna cuenta de administrador deshabilitada para desbloquear.");
    add("恢复文件与当前请求或账号库代次不匹配", "Recovery file does not match the current request or database generation.", "Файл восстановления не соответствует запросу или поколению базы.", "Wiederherstellungsdatei passt nicht zur Anforderung oder Datenbankgeneration.", "Le fichier de récupération ne correspond pas à la demande ou à la génération.", "El archivo de recuperación no coincide con la solicitud o generación actual.");
    add("恢复已完成，账号可重新登录", "Recovery completed. The account can sign in again.", "Восстановление выполнено, вход снова доступен.", "Wiederherstellung abgeschlossen. Anmeldung wieder möglich.", "Récupération terminée. Le compte peut se reconnecter.", "Recuperación completada. La cuenta puede iniciar sesión de nuevo.");
    add("账号已保存", "Account saved", "Учётная запись сохранена", "Konto gespeichert", "Compte enregistré", "Cuenta guardada");
    add("账号已删除", "Account deleted", "Учётная запись удалена", "Konto gelöscht", "Compte supprimé", "Cuenta eliminada");

    // Recovery & License
    add("管理员离线恢复", "Offline admin recovery", "Восстановление администратора", "Offline-Adminwiederherstellung", "Récupération administrateur hors ligne", "Recuperación de administrador sin conexión");
    add("电脑名称", "Computer name", "Имя компьютера", "Computername", "Nom de l’ordinateur", "Nombre del equipo");
    add("未获取电脑名称", "Computer name unavailable", "Имя компьютера недоступно", "Computername nicht verfügbar", "Nom de l’ordinateur indisponible", "Nombre del equipo no disponible");
    add("账号库代次", "Account database generation", "Поколение базы аккаунтов", "Generation der Kontendatenbank", "Génération de la base de comptes", "Generación de base de cuentas");
    add("当前请求", "Current request", "Текущий запрос", "Aktuelle Anforderung", "Demande actuelle", "Solicitud actual");
    add("尚未生成", "Not generated", "Ещё не создан", "Noch nicht erstellt", "Non générée", "Sin generar");
    add("导出请求文件", "Export request file", "Экспортировать запрос", "Anfrage exportieren", "Exporter la demande", "Exportar solicitud");
    add("导入恢复文件", "Import recovery file", "Импорт файла восстановления", "Wiederherstellungsdatei importieren", "Importer le fichier de récupération", "Importar archivo de recuperación");
    add("签发恢复文件", "Issue recovery file", "Выдача файла восстановления", "Wiederherstellungsdatei ausstellen", "Délivrer un fichier de récupération", "Emitir archivo de recuperación");
    add("先导入目标电脑导出的请求文件。", "Import the request exported by the target computer.", "Импортируйте запрос с целевого компьютера.", "Importieren Sie die Anfrage vom Zielcomputer.", "Importez la demande exportée par le poste cible.", "Importe la solicitud exportada por el equipo de destino.");
    add("目标电脑", "Target computer", "Целевой компьютер", "Zielcomputer", "Poste cible", "Equipo de destino");
    add("请求编号", "Request ID", "Номер запроса", "Anfrage-ID", "Identifiant de demande", "ID de solicitud");
    add("导入请求文件", "Import request file", "Импортировать запрос", "Anfrage importieren", "Importer la demande", "Importar solicitud");
    add("导出恢复文件", "Export recovery file", "Экспортировать файл восстановления", "Wiederherstellungsdatei exportieren", "Exporter le fichier de récupération", "Exportar archivo de recuperación");
    add("许可文件", "License file", "Файл лицензии", "Lizenzdatei", "Fichier de licence", "Archivo de licencia");
    add("当前状态", "Current state", "Текущее состояние", "Aktueller Zustand", "État actuel", "Estado actual");
    add("有效期", "Validity", "Срок действия", "Gültigkeit", "Validité", "Vigencia");
    add("导入许可文件", "Import license file", "Импортировать лицензию", "Lizenzdatei importieren", "Importer la licence", "Importar licencia");
    add("清除许可", "Clear license", "Удалить лицензию", "Lizenz entfernen", "Effacer la licence", "Eliminar licencia");
    add("确认清除", "Confirm removal", "Подтвердить удаление", "Entfernen bestätigen", "Confirmer la suppression", "Confirmar eliminación");
    add("清除后，工作站将进入许可失效状态。重新启用需要申请新许可。", "Clearing the license deactivates this workstation. Request a new license to reactivate it.", "Удаление лицензии деактивирует станцию. Для активации запросите новую лицензию.", "Das Entfernen der Lizenz deaktiviert den Arbeitsplatz. Zur Aktivierung eine neue Lizenz beantragen.", "Effacer la licence désactive le poste. Demandez une nouvelle licence pour le réactiver.", "Eliminar la licencia desactiva la estación. Solicite una licencia nueva para reactivarla.");
    add("许可已失效", "License inactive", "Лицензия недействительна", "Lizenz inaktiv", "Licence inactive", "Licencia inactiva");
    add("许可有效时，由管理员或调试员管理许可文件。", "An administrator or debugger manages active licenses.", "Действующими лицензиями управляет администратор или наладчик.", "Aktive Lizenzen werden vom Administrator oder Inbetriebnehmer verwaltet.", "Les licences actives sont gérées par un administrateur ou un technicien de mise au point.", "Las licencias activas se gestionan desde una cuenta de administración o depuración.");
    add("签发许可文件", "Issue license file", "Выдача лицензии", "Lizenz ausstellen", "Délivrer une licence", "Emitir licencia");
    add("按年数", "By number of years", "По количеству лет", "Nach Anzahl der Jahre", "Par nombre d’années", "Por número de años");
    add("按截止日期", "By expiration date", "По дате окончания", "Nach Ablaufdatum", "Par date d’expiration", "Por fecha de vencimiento");
    add("有效年数", "Validity in years", "Срок в годах", "Gültigkeit in Jahren", "Durée en années", "Validez en años");
    add("截止日期（UTC）", "Expiration date (UTC)", "Дата окончания (UTC)", "Ablaufdatum (UTC)", "Date d’expiration (UTC)", "Fecha de vencimiento (UTC)");
    add("导出许可文件", "Export license file", "Экспортировать лицензию", "Lizenzdatei exportieren", "Exporter la licence", "Exportar licencia");
    add("工作站许可已启用", "Workstation license activated", "Лицензия рабочей станции активирована", "Arbeitsplatzlizenz aktiviert", "Licence du poste activée", "Licencia de la estación activada");
    add("请输入有效的许可年数。", "Enter a valid number of years.", "Введите допустимое количество лет.", "Geben Sie eine gültige Anzahl von Jahren ein.", "Saisissez un nombre d’années valide.", "Introduzca un número de años válido.");
    add("许可有效期无效或已到期", "The license expiration date is invalid or has passed.", "Дата окончания лицензии недействительна или уже прошла.", "Das Ablaufdatum der Lizenz ist ungültig oder liegt in der Vergangenheit.", "La date d’expiration de la licence est invalide ou dépassée.", "La fecha de vencimiento de la licencia no es válida o ya pasó.");
    add("请选择对应的请求文件", "Select the matching request file.", "Выберите соответствующий файл запроса.", "Passende Anfragedatei auswählen.", "Sélectionnez le fichier de demande correspondant.", "Seleccione el archivo de solicitud correspondiente.");
    add("请选择许可请求文件。", "Select a license request file.", "Выберите файл запроса лицензии.", "Wählen Sie eine Lizenzanfrage aus.", "Sélectionnez un fichier de demande de licence.", "Seleccione un archivo de solicitud de licencia.");
    add("请选择账号恢复请求文件。", "Select an account recovery request file.", "Выберите файл запроса восстановления учётной записи.", "Wählen Sie eine Anfrage zur Kontowiederherstellung aus.", "Sélectionnez un fichier de demande de récupération de compte.", "Seleccione un archivo de solicitud de recuperación de cuenta.");
    add("请在目标电脑重新导出新版请求文件。", "Export a new request from the target computer.", "Экспортируйте новый запрос с целевого компьютера.", "Exportieren Sie eine neue Anfrage vom Zielcomputer.", "Exportez une nouvelle demande depuis le poste cible.", "Exporte una nueva solicitud desde el equipo de destino.");
    add("请求文件内容不完整或已损坏。", "The request file is incomplete or damaged.", "Файл запроса неполный или повреждён.", "Die Anfragedatei ist unvollständig oder beschädigt.", "Le fichier de demande est incomplet ou endommagé.", "El archivo de solicitud está incompleto o dañado.");
    add("无法读取文件，请重新选择本地文件。", "Could not read the file. Select a local file again.", "Не удалось прочитать файл. Выберите локальный файл снова.", "Die Datei konnte nicht gelesen werden. Wählen Sie erneut eine lokale Datei aus.", "Impossible de lire le fichier. Sélectionnez de nouveau un fichier local.", "No se pudo leer el archivo. Seleccione de nuevo un archivo local.");
    add("文件与本机当前请求不匹配", "The file does not match this workstation’s current request.", "Файл не соответствует текущему запросу этой станции.", "Die Datei passt nicht zur aktuellen Anfrage dieses Arbeitsplatzes.", "Le fichier ne correspond pas à la demande en cours sur ce poste.", "El archivo no coincide con la solicitud actual de esta estación.");
    add("请选择由新版请求签发的对应文件", "Select a file issued for a current-format request.", "Выберите файл, выданный по запросу нового формата.", "Eine für das aktuelle Anfrageformat ausgestellte Datei auswählen.", "Sélectionnez un fichier délivré pour une demande au format actuel.", "Seleccione un archivo emitido para una solicitud del formato actual.");
    add("需要调试员权限", "Debugger privileges required.", "Требуются права наладчика.", "Inbetriebnahmerechte erforderlich.", "Les droits de mise au point sont requis.", "Se requieren permisos de depuración.");
    add("请先登录", "Sign in first", "Сначала войдите", "Bitte zuerst anmelden", "Connectez-vous d’abord", "Inicie sesión primero");
    add("请激活模拟许可", "Please activate the simulated license.", "Активируйте модельную лицензию.", "Bitte aktivieren Sie die simulierte Lizenz.", "Veuillez activer la licence simulée.", "Active la licencia simulada.");
    add("请选择小于 256 KB 的演示 JSON 文件", "Select a demo JSON file smaller than 256 KB.", "Выберите демофайл JSON размером менее 256 КБ.", "Wählen Sie eine Demo-JSON-Datei unter 256 KB.", "Sélectionnez un fichier JSON de démo inférieur à 256 Ko.", "Seleccione un archivo JSON de demostración menor de 256 KB.");
    add("文件不是有效的 JSON", "The file is not valid JSON.", "Файл не является корректным JSON.", "Die Datei ist kein gültiges JSON.", "Le fichier n’est pas un JSON valide.", "El archivo no es un JSON válido.");
    add("无法写入文件，请检查路径权限。", "Could not write the file. Check path permissions.", "Не удалось записать файл. Проверьте права доступа.", "Datei konnte nicht geschrieben werden. Berechtigungen prüfen.", "Impossible d’écrire le fichier. Vérifiez les permissions.", "No se pudo escribir el archivo. Compruebe los permisos.");
    add("导出许可请求", "Exported license request", "Экспортирован запрос лицензии", "Lizenzanfrage exportiert", "Demande de licence exportée", "Solicitud de licencia exportada");
    add("导出账号恢复请求", "Exported account recovery request", "Экспортирован запрос восстановления", "Wiederherstellungsanfrage exportiert", "Demande de récupération exportée", "Solicitud de recuperación exportada");
    add("已导出设置", "Settings exported", "Настройки экспортированы", "Einstellungen exportiert", "Paramètres exportés", "Configuración exportada");
    add("已导出评审清单", "Review manifest exported", "Перечень проверки экспортирован", "Prüfliste exportiert", "Manifeste de revue exporté", "Lista de revisión exportada");
    add("已记录模板复核", "Template review recorded", "Проверка шаблона записана", "Vorlagenprüfung erfasst", "Revue du modèle enregistrée", "Revisión de plantilla registrada");

    // Templates & About
    add("当前模板", "Current template", "Текущий шаблон", "Aktuelle Vorlage", "Modèle actuel", "Plantilla actual");
    add("适用板型", "Supported board", "Совместимая плата", "Kompatible Baugruppe", "Carte compatible", "Placa compatible");
    add("评审状态", "Review status", "Статус проверки", "Prüfstatus", "État de revue", "Estado de revisión");
    add("记录模板复核", "Record template review", "Записать проверку шаблона", "Vorlagenprüfung erfassen", "Consigner la revue du modèle", "Registrar revisión de plantilla");
    add("导出评审清单", "Export review manifest", "Экспорт перечня проверки", "Prüfliste exportieren", "Exporter le manifeste de revue", "Exportar lista de revisión");
    add("版本与兼容性", "Versions and compatibility", "Версии и совместимость", "Versionen und Kompatibilität", "Versions et compatibilité", "Versiones y compatibilidad");
    add("组件", "Component", "Компонент", "Komponente", "Composant", "Componente");
    add("版本 / 标识", "Version / identifier", "Версия / ID", "Version / Kennung", "Version / identifiant", "Versión / ID");
    add("适用范围", "Scope", "Область применения", "Geltungsbereich", "Portée", "Ámbito");
    add("桌面兼容基线", "Desktop compatibility baseline", "Базовая совместимость ПО", "Desktop-Kompatibilitätsbasis", "Référence de compatibilité bureau", "Base de compatibilidad de escritorio");
    add("管理 API", "Management API", "API управления", "Verwaltungs-API", "API de gestion", "API de gestión");
    add("配置模型", "Configuration model", "Модель конфигурации", "Konfigurationsmodell", "Modèle de configuration", "Modelo de configuración");
    add("流程 IR", "Workflow IR", "IR процесса", "Ablauf-IR", "RI de séquence", "RI de secuencia");
    add("模拟模板", "Simulated template", "Модельный шаблон", "Simulationsvorlage", "Modèle simulé", "Plantilla simulada");
    add("信任目录", "Trust catalog", "Каталог доверия", "Vertrauenskatalog", "Catalogue de confiance", "Catálogo de confianza");
    add("目录域", "Catalog domain", "Домен каталога", "Katalogdomäne", "Domaine du catalogue", "Dominio del catálogo");
    add("仅演示", "Demo only", "Только демонстрация", "Nur Demo", "Démo seulement", "Solo demostración");
    add("模板发布", "Template release", "Выпуск шаблона", "Vorlagenfreigabe", "Publication du modèle", "Publicación de plantilla");
    add("演示模板评审", "Demo template review", "Проверка демошаблона", "Prüfung der Demovorlage", "Revue du modèle de démo", "Revisión de plantilla de demostración");
    add("签发根", "Signing root", "Корень подписи", "Signaturwurzel", "Racine de signature", "Raíz de firma");
    add("原型样例根", "Prototype sample root", "Образец корня прототипа", "Beispielwurzel des Prototyps", "Racine d’exemple du prototype", "Raíz de ejemplo del prototipo");
    add("部署许可", "Deployment authorization", "Разрешение на развёртывание", "Bereitstellungsfreigabe", "Autorisation de déploiement", "Permiso de despliegue");
    add("未获生产签发", "No production signature", "Нет производственной подписи", "Keine Produktionssignatur", "Sans signature de production", "Sin firma de producción");
    add("查看信任记录", "View trust record", "Просмотр записи доверия", "Vertrauenseintrag anzeigen", "Afficher la fiche de confiance", "Ver registro de confianza");
    add("收起详情", "Hide details", "Скрыть подробности", "Details ausblenden", "Masquer les détails", "Ocultar detalles");
    add("SIM-ROOT-01 只识别演示文件类型。正式发行由离线签发方提供凭据。",
        "SIM-ROOT-01 recognizes demo file types only. Production releases use credentials issued by an offline authority.",
        "SIM-ROOT-01 распознаёт только демонстрационные типы файлов. Для промышленных выпусков требуются учётные данные автономного центра.",
        "SIM-ROOT-01 erkennt nur Demo-Dateitypen. Produktionsfreigaben verwenden Nachweise einer Offline-Ausgabestelle.",
        "SIM-ROOT-01 reconnaît uniquement les types de fichiers de démonstration. Les versions de production utilisent des justificatifs hors ligne.",
        "SIM-ROOT-01 solo reconoce tipos de archivo de demostración. Las versiones de producción usan credenciales de una autoridad sin conexión.");
    add("真实硬件绑定、板卡管理接口与生产签发根当前属于无法确认项，本模块仅运行本地工作站仿真（SIM-ROOT-01）。",
        "Real hardware binding, board management API, and production trust root are unconfirmed; this module runs local workstation simulation (SIM-ROOT-01) only.",
        "Привязка к оборудованию, API управления платой и промышленный корень доверия не подтверждены; модуль работает только в режиме локальной симуляции (SIM-ROOT-01).",
        "Echte Hardwarebindung, Board-Verwaltungs-API und Produktions-Vertrauenswurzel sind unbestätigt; dieses Modul läuft nur als lokale Simulation (SIM-ROOT-01).",
        "La liaison matérielle réelle, l’API de gestion de carte et la racine de production ne sont pas confirmées ; ce module s’exécute uniquement en simulation locale (SIM-ROOT-01).",
        "La vinculación real de hardware, la API de placa y la raíz de producción no están confirmadas; este módulo funciona solo como simulación local (SIM-ROOT-01).");
}

QString I18nCatalog::lang() const
{
    return m_lang;
}

bool I18nCatalog::setLang(const QString &lang)
{
    if (!isSupportedLanguage(lang)) {
        return false;
    }
    if (m_lang == lang) {
        return true;
    }
    m_lang = lang;
    ++m_revision;
    emit langChanged();
    return true;
}

int I18nCatalog::revision() const
{
    return m_revision;
}

QVariantList I18nCatalog::languageOptions() const
{
    return {
        QVariantMap{{QStringLiteral("value"), QStringLiteral("zh-CN")}, {QStringLiteral("label"), QStringLiteral("简体中文")}},
        QVariantMap{{QStringLiteral("value"), QStringLiteral("en")}, {QStringLiteral("label"), QStringLiteral("English")}},
        QVariantMap{{QStringLiteral("value"), QStringLiteral("ru")}, {QStringLiteral("label"), QStringLiteral("Русский")}},
        QVariantMap{{QStringLiteral("value"), QStringLiteral("de")}, {QStringLiteral("label"), QStringLiteral("Deutsch")}},
        QVariantMap{{QStringLiteral("value"), QStringLiteral("fr")}, {QStringLiteral("label"), QStringLiteral("Français")}},
        QVariantMap{{QStringLiteral("value"), QStringLiteral("es")}, {QStringLiteral("label"), QStringLiteral("Español")}},
    };
}

bool I18nCatalog::isSupportedLanguage(const QString &lang)
{
    return lang == QLatin1String("zh-CN")
        || lang == QLatin1String("en")
        || lang == QLatin1String("ru")
        || lang == QLatin1String("de")
        || lang == QLatin1String("fr")
        || lang == QLatin1String("es");
}

QStringList I18nCatalog::supportedLanguages()
{
    return {
        QStringLiteral("zh-CN"),
        QStringLiteral("en"),
        QStringLiteral("ru"),
        QStringLiteral("de"),
        QStringLiteral("fr"),
        QStringLiteral("es")
    };
}

QString I18nCatalog::translateInternal(const QString &zhSource, int langIdx, int depth) const
{
    if (langIdx < 0 || langIdx >= 5 || depth > 3) {
        return zhSource;
    }
    const QString trimmed = zhSource.trimmed();
    auto it = m_dict.constFind(trimmed);
    if (it != m_dict.constEnd()) {
        return it.value()[static_cast<size_t>(langIdx)];
    }
    // Support compound strings joined by " · "
    const QString sep = QStringLiteral(" · ");
    if (trimmed.contains(sep)) {
        const QStringList parts = trimmed.split(sep);
        QStringList translatedParts;
        translatedParts.reserve(parts.size());
        for (const QString &part : parts) {
            translatedParts.append(translateInternal(part, langIdx, depth + 1));
        }
        return translatedParts.join(sep);
    }
    return zhSource;
}

QString I18nCatalog::tr(const QString &zhSource, const QString &arg0) const
{
    const int idx = languageIndex(m_lang);
    QString result = translateInternal(zhSource, idx, 0);
    if (!arg0.isNull() && result.contains(QStringLiteral("{0}"))) {
        const QString translatedArg = translateInternal(arg0, idx, 1);
        result.replace(QStringLiteral("{0}"), translatedArg);
    }
    return result;
}

QString I18nCatalog::t(const QString &zhSource, int rev, const QString &arg0) const
{
    Q_UNUSED(rev)
    return tr(zhSource, arg0);
}

} // namespace FireSystem
