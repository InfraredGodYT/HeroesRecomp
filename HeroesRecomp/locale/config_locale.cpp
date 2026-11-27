#include <user/config.h>

/*
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! LOCALISATION NOTES !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    - Ensure brand names are always presented on the same line.

      Correct:
      This is a string that contains a brand name like
      Xbox, which is one of the few consoles to have a port of
      Shadow the Hedgehog.

      Incorrect:
      This is a string that contains a brand name like Xbox
      which is one of the few consoles to have a port of Shadow
      Hedgehog.

    - Ensure your locale is added in the correct order following the language enum.

      Correct:
      {
          { ELanguage::English,  "Example" },
          { ELanguage::Japanese, "Example" },
          { ELanguage::German,   "Example" },
          { ELanguage::French,   "Example" },
          { ELanguage::Spanish,  "Example" },
          { ELanguage::Italian,  "Example" }
      }

      Incorrect:
      {
          { ELanguage::English,  "Example" },
          { ELanguage::French,   "Example" },
          { ELanguage::Spanish,  "Example" },
          { ELanguage::German,   "Example" },
          { ELanguage::Italian,  "Example" },
          { ELanguage::Japanese, "Example" }
      }

    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#define CONFIG_DEFINE_LOCALE(name) \
    CONFIG_LOCALE g_##name##_locale =

#define CONFIG_DEFINE_ENUM_LOCALE(type) \
    CONFIG_ENUM_LOCALE(type) g_##type##_locale =


CONFIG_DEFINE_LOCALE(Language)
{
    { ELanguage::English, { "Language", "Change the language used for text." } },
    { ELanguage::Japanese, { "言語", "テキストで表示される言語を変更します。" } },
    { ELanguage::German,  { "Sprache", "Ändere die verwendete Sprache für text." } },
    { ELanguage::French,   { "Langue", "Modifie la langue utilisée pour le texte." } },
    { ELanguage::Spanish,  { "Idioma", "Cambia el idioma de los textos." } },
    { ELanguage::Italian,  { "Linguaggio", "Cambia la lingua usata per il testo" } },
};

// Notes: do not localise this.
CONFIG_DEFINE_ENUM_LOCALE(ELanguage)
{
    {
        ELanguage::English,
        {
            { ELanguage::English,  { "ENGLISH", "" } },
            { ELanguage::Japanese, { "日本語", "" } },
            { ELanguage::German,   { "DEUTSCH", "" } },
            { ELanguage::French,   { "FRANÇAIS", "" } },
            { ELanguage::Spanish,  { "ESPAÑOL", "" } },
            { ELanguage::Italian,  { "ITALIANO", "" } }
        }
    }
};

CONFIG_DEFINE_ENUM_LOCALE(EVoiceLanguage)
{
    {
        ELanguage::English,
        {
            { EVoiceLanguage::English,  { "ENGLISH", "" } },
            { EVoiceLanguage::Japanese, { "JAPANESE", "" } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { EVoiceLanguage::English,  { "英語", "" } },
            { EVoiceLanguage::Japanese, { "日本語", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { EVoiceLanguage::English,  { "ENGLISCH", "" } },
            { EVoiceLanguage::Japanese, { "JAPANISCH", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EVoiceLanguage::English,  { "ANGLAIS", "" } },
            { EVoiceLanguage::Japanese, { "JAPONAIS", "" } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { EVoiceLanguage::English,  { "INGLÉS", "" } },
            { EVoiceLanguage::Japanese, { "JAPONÉS", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EVoiceLanguage::English,  { "INGLESE", "" } },
            { EVoiceLanguage::Japanese, { "GIAPPONESE", "" } }
        }
    },
};

CONFIG_DEFINE_LOCALE(Hints)
{
    { ELanguage::English,  { "Hints", "Show hints during gameplay." } },
    { ELanguage::Japanese, { "ヒントリング", "ゲーム内にヒントリングを表示するか選択できます。" } },
    { ELanguage::German,   { "Hinweise", "Zeige Hinweise während des Spiels." } },
    { ELanguage::French,   { "Indices", "Affiche les indices pendant le jeu." } },
    { ELanguage::Spanish,  { "Pistas", "Muestra pistas durante el juego." } },
    { ELanguage::Italian,  { "Indizi", "Mostra degli indizzi durante il gioco." } }
};

CONFIG_DEFINE_LOCALE(AchievementNotifications)
{
    { ELanguage::English,  { "Achievement Notifications", "Show notifications for unlocking achievements. Achievements will still be rewarded with notifications disabled." } },
    { ELanguage::Japanese, { "実績通知", "実績を解除したときに通知を表示します。通知を無効にしても、実績は獲得できます。" } },
    { ELanguage::German,   { "Erfolgsbenachrichtigungen", "Zeige Benachrichtigungen für das Freischalten von Erfolgen. Erfolge werden weiterhin freigeschaltet, auch wenn die Benachrichtigungen ausgeschaltet sind." } },
    { ELanguage::French,   { "Notification des succès", "Affiche les notifications pour le déverrouillage des succès. Les succès seront toujours obtenus même si les notifications sont désactivées." } },
    { ELanguage::Spanish,  { "Notificaciones de logros", "Muestra una notificación al desbloquear un logro. Los logros se seguirán desbloqueando aunque las notificaciones estén desactivadas." } },
    { ELanguage::Italian,  { "Notifiche obiettivi", "Mostra delle notifiche quando sblocchi degli obiettivi. Gli obiettivi verranno comunque assegnati anche con le notifiche disattivate." } }
};

CONFIG_DEFINE_LOCALE(ConfirmationPrompt)
{
    { ELanguage::English,  { "Confirmation Prompts", "Shows confirmation prompts when loading a save file, exiting to the menu, restarting, and in other similar situations. Disabling this option replicates the behavior of the GameCube version." } },
    { ELanguage::Japanese, { "確認プロンプト", "ゲームデータのロード、「やめる」、「はじめから」などの際に確認プロンプトを表示します。このオプションを無効にすると、ゲームキューブ版と同じ動作になります。" } },
    { ELanguage::German,   { "Bestätigungsaufforderungen", "Zeigt Bestätigungsabfragen beim Laden eines Spielstands, beim Verlassen des Menüs, beim Neustart und in ähnlichen Situationen an. Durch Deaktivieren dieser Option wird das Verhalten der GameCube-Version Repliziert." } },
    { ELanguage::French,   { "Message de confirmation", "Affiche les messages de confirmation lors du chargement d'une sauvegarde, du retour au menu, du redémarrage et dans d'autres situations similaires. Désactiver cette option reproduit le comportement de la version GameCube." } },
    { ELanguage::Spanish,  { "Mensajes de confirmación", "Muestra una confirmación al cargar un archivo de guardado, salir al menú principal, reiniciar el nivel y otras situaciones similares. Desactivar esta opción replica el funcionamiento de la versión de GameCube." } },
    { ELanguage::Italian,  { "Prompt di Conferma", "Mostra un Prompt di Conferma mentre carichi un salvataggio, ritornando al menu, riavviando il gioco, e altre simili situazioni. Disabilitare questa opzione replica il comportamento della versione GameCube." } },
};

CONFIG_DEFINE_LOCALE(ControllerIcons)
{
    { ELanguage::English,  { "Controller Icons", "Change the icons to match your controller." } },
    { ELanguage::Japanese, { "コントローラーアイコン", "お使いのコントローラーに合わせてアイコンを変更します。" } },
    { ELanguage::German,   { "Controllersymbole", "Ändere die Controllersymbole, um sie auf dein Modell anzupassen." } },
    { ELanguage::French,   { "Icône des boutons", "Modifie les icônes pour les faire correspondre à votre manette." } },
    { ELanguage::Spanish,  { "Iconos del mando", "Cambia los iconos para que coincidan con tu mando." } },
    { ELanguage::Italian,  { "Icone dei tasti", "Modifica le icone per farle corrispondere con il tuo controller." } }
};


CONFIG_DEFINE_ENUM_LOCALE(EControllerIcons)
{
    {
        ELanguage::English,
        {
            { EControllerIcons::Auto,        { "AUTO", "The game will determine which icons to use based on the current input device." } },
            { EControllerIcons::Xbox,        { "XBOX", "" } },
            { EControllerIcons::PlayStation2, { "PS2", "" } },
            { EControllerIcons::Gamecube, { "GAMECUBE", "" } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { EControllerIcons::Auto,        { "自動", "現在の操作デバイスに応じて、ゲームが使用するアイコンを決定します。" } },
            { EControllerIcons::Xbox,        { "XBOX", "" } },
            { EControllerIcons::PlayStation2, { "PS2", "" } },
            { EControllerIcons::Gamecube, { "GAMECUBE", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { EControllerIcons::Auto,        { "AUTO", "Das Spiel erkennt automatisch deinen Controller um die Symbole dementsprechend anzupassen." } },
            { EControllerIcons::Xbox,        { "XBOX", "" } },
            { EControllerIcons::PlayStation2, { "PS2", "" } },
            { EControllerIcons::Gamecube, { "GAMECUBE", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EControllerIcons::Auto,        { "AUTO", "Le jeu déterminera automatiquement quelles icônes utiliser en fonction du périphérique d'entrée." } },
            { EControllerIcons::Xbox,        { "XBOX", "" } },
            { EControllerIcons::PlayStation2, { "PS2", "" } },
            { EControllerIcons::Gamecube, { "GAMECUBE", "" } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { EControllerIcons::Auto,        { "AUTO", "El juego determinará de forma automática qué iconos utilizar dependiendo del dispositivo de entrada." } },
            { EControllerIcons::Xbox,        { "XBOX", "" } },
            { EControllerIcons::PlayStation2, { "PS2", "" } },
            { EControllerIcons::Gamecube, { "GAMECUBE", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EControllerIcons::Auto,        { "AUTO", "Auto: il gioco determinerà quali icone da utilizzare in base al dispositivo di input attuale." } },
            { EControllerIcons::Xbox,        { "XBOX", "" } },
            { EControllerIcons::PlayStation2, { "PS2", "" } },
            { EControllerIcons::Gamecube, { "GAMECUBE", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(HorizontalCamera1P)
{
    { ELanguage::English, { "1P Horizontal Camera", "Change how the camera moves left and right." } },
    { ELanguage::Japanese, { "1Pカメラの左右", "カメラ左右の動く方向を選択できます。" } },
    { ELanguage::German,   { "1SP Horizontale Kamera", "Ändere wie sich die Kamera nach links und rechts bewegt." } },
    { ELanguage::French,   { "J1 Caméra horizontale", "Modifie la rotation horizontale de la caméra." } },
    { ELanguage::Spanish,  { "J1 Cámara Horizontal", "Selecciona la dirección horizontal de la cámara del Jugador 1." } },
    { ELanguage::Italian,  { "1P Telecamera orizzontale", "Modifica come la telecamera si muove da sinistra a destra." } }
};

CONFIG_DEFINE_LOCALE(HorizontalCamera2P)
{
    { ELanguage::English, { "2P Horizontal Camera", "Change how the camera moves left and right." } },
    { ELanguage::Japanese, { "2Pカメラの左右", "カメラ左右の動く方向を選択できます。" } },
    { ELanguage::German,   { "2SP Horizontale Kamera", "Ändere wie sich die Kamera nach links und rechts bewegt." } },
    { ELanguage::French,   { "J2 Caméra horizontale", "Modifie la rotation horizontale de la caméra." } },
    { ELanguage::Spanish,  { "J2 Cámara Horizontal", "Selecciona la dirección horizontal de la cámara del Jugador 2." } },
    { ELanguage::Italian,  { "2P Telecamera orizzontale", "Modifica come la telecamera si muove da sinistra a destra." } }
};

CONFIG_DEFINE_LOCALE(HorizontalCamera3P)
{
    { ELanguage::English, { "3P Horizontal Camera", "Change how the camera moves left and right." } },
    { ELanguage::Japanese, { "3Pカメラの左右", "カメラ左右の動く方向を選択できます。" } },
    { ELanguage::German,   { "3SP Horizontale Kamera", "Ändere wie sich die Kamera nach links und rechts bewegt." } },
    { ELanguage::French,   { "J3 Caméra horizontale", "Modifie la rotation horizontale de la caméra." } },
    { ELanguage::Spanish,  { "J3 Cámara Horizontal", "Selecciona la dirección horizontal de la cámara del Jugador 3." } },
    { ELanguage::Italian,  { "3P Telecamera orizzontale", "Modifica come la telecamera si muove da sinistra a destra." } }
};

CONFIG_DEFINE_LOCALE(HorizontalCamera4P)
{
    { ELanguage::English, { "4P Horizontal Camera", "Change how the camera moves left and right." } },
    { ELanguage::Japanese, { "4Pカメラの左右", "カメラ左右の動く方向を選択できます。" } },
    { ELanguage::German,   { "4SP Horizontale Kamera", "Ändere wie sich die Kamera nach links und rechts bewegt." } },
    { ELanguage::French,   { "J4 Caméra horizontale", "Modifie la rotation horizontale de la caméra." } },
    { ELanguage::Spanish,  { "J4 Cámara Horizontal", "Selecciona la dirección horizontal de la cámara del Jugador 4." } },
    { ELanguage::Italian,  { "4P Telecamera orizzontale", "Modifica come la telecamera si muove da sinistra a destra." } }
};


CONFIG_DEFINE_ENUM_LOCALE(ECameraRotationMode)
{
    {
        ELanguage::English,
        {
            { ECameraRotationMode::Normal,  { "NORMAL", "" } },
            { ECameraRotationMode::Reverse, { "REVERSE", "" } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { ECameraRotationMode::Normal,  { "ノーマル", "" } },
            { ECameraRotationMode::Reverse, { "リバース", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { ECameraRotationMode::Normal,  { "NORMAL", "" } },
            { ECameraRotationMode::Reverse, { "INVERTIERT", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { ECameraRotationMode::Normal,  { "NORMALE", "" } },
            { ECameraRotationMode::Reverse, { "INVERSÉE", "" } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { ECameraRotationMode::Normal,  { "NORMAL", "" } },
            { ECameraRotationMode::Reverse, { "INVERTIDA", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { ECameraRotationMode::Normal,  { "NORMALE", "" } },
            { ECameraRotationMode::Reverse, { "INVERTITA", "" } }
        }
    }
};



CONFIG_DEFINE_LOCALE(Vibration1P)
{
    { ELanguage::English,  { "1P Vibration", "Toggle controller vibration." } },
    { ELanguage::Japanese, { "1P振動", "コントローラの振動機能を切り替えます。" } },
    { ELanguage::German,   { "1SP Vibration", "Ändere die Vibrations einstellungen des Controllers." } },
    { ELanguage::French,   { "J1 Vibration", "Active les vibrations de la manette." } },
    { ELanguage::Spanish,  { "J1 Vibración", "Activa o desactiva la vibración del mando del Jugador 1." } },
    { ELanguage::Italian,  { "1P Vibrazione", "Attiva/disattiva la vibrazione del controller." } }
};

CONFIG_DEFINE_LOCALE(Vibration2P)
{
    { ELanguage::English,  { "2P Vibration", "Toggle controller vibration." } },
    { ELanguage::Japanese, { "2P振動", "コントローラの振動機能を切り替えます。" } },
    { ELanguage::German,   { "2SP Vibration", "Ändere die Vibrations einstellungen des Controllers." } },
    { ELanguage::French,   { "J2 Vibration", "Active les vibrations de la manette." } },
    { ELanguage::Spanish,  { "J2 Vibración", "Activa o desactiva la vibración del mando del Jugador 2." } },
    { ELanguage::Italian,  { "2P Vibrazione", "Attiva/disattiva la vibrazione del controller." } }
};

CONFIG_DEFINE_LOCALE(Vibration3P)
{
    { ELanguage::English,  { "3P Vibration", "Toggle controller vibration." } },
    { ELanguage::Japanese, { "3P振動", "コントローラの振動機能を切り替えます。" } },
    { ELanguage::German,   { "3SP Vibration", "Ändere die Vibrations einstellungen des Controllers." } },
    { ELanguage::French,   { "J3 Vibration", "Active les vibrations de la manette." } },
    { ELanguage::Spanish,  { "J3 Vibración", "Activa o desactiva la vibración del mando del Jugador 3." } },
    { ELanguage::Italian,  { "3P Vibrazione", "Attiva/disattiva la vibrazione del controller." } }
};

CONFIG_DEFINE_LOCALE(Vibration4P)
{
    { ELanguage::English,  { "4P Vibration", "Toggle controller vibration." } },
    { ELanguage::Japanese, { "4P振動", "コントローラの振動機能を切り替えます。" } },
    { ELanguage::German,   { "4SP Vibration", "Ändere die Vibrations einstellungen des Controllers." } },
    { ELanguage::French,   { "J4 Vibration", "Active les vibrations de la manette." } },
    { ELanguage::Spanish,  { "J4 Vibración", "Activa o desactiva la vibración del mando del Jugador 4." } },
    { ELanguage::Italian,  { "4P Vibrazione", "Attiva/disattiva la vibrazione del controller." } }
};

CONFIG_DEFINE_LOCALE(AllowBackgroundInput)
{
    { ELanguage::English,  { "Allow Background Input", "Allow controller input while the game window is unfocused." } },
    { ELanguage::Japanese, { "バックグラウンド操作", "ウィンドウが非アクティブの時も、コントローラー入力を受け付けます。" } },
    { ELanguage::German,   { "Erlaube Hintergrundeingaben", "Erlaube Eingaben deines Controllers auch wenn das Spielfenster nicht fokussiert ist." } },
    { ELanguage::French,   { "Manette en arrière plan", "Permet d'utiliser la manette dans le jeu lorsque qu'il n'est pas au premier plan." } },
    { ELanguage::Spanish,  { "Control en segundo plano", "Permite controlar el juego con un mando mientras la ventana esté en segundo plano." } },
    { ELanguage::Italian,  { "Input con la finestra inattiva", "Attiva/disattiva i tasti del controller mentre la finestra è inattiva." } }
};

CONFIG_DEFINE_LOCALE(StickDeadzone1P)
{
    { ELanguage::English,  { "Stick Deadzone", "Adjust analog deadzone." } },
    { ELanguage::Japanese, { "スティックのデッドゾーン", "アナログスティックのデッドゾーンを調整します。" } },
    { ELanguage::German,   { "Stick Totzone", "Analoge Totzone anpassen." } },
    { ELanguage::French,   { "Zone morte du Stick", "Ajuster la zone morte analogique." } },
    { ELanguage::Spanish,  { "Zona muerta del stick", "Ajusta la zona muerta del stick." } },
    { ELanguage::Italian,  { "Zona morta dello Stick", "Regola la zona morta dello stick analogico." } }
};

// shared by all players
CONFIG_LOCALE g_StickDeadzone2P_locale = g_StickDeadzone1P_locale;
CONFIG_LOCALE g_StickDeadzone3P_locale = g_StickDeadzone1P_locale;
CONFIG_LOCALE g_StickDeadzone4P_locale = g_StickDeadzone1P_locale;

CONFIG_DEFINE_LOCALE(StickSensitivity1P)
{
    { ELanguage::English,  { "Stick Sensitivity", "Adjust analog sensitivity." } },
    { ELanguage::Japanese, { "スティック感度", "アナログスティックの感度を調整します。" } },
    { ELanguage::German,   { "Stick empfindlichkeit", "Analoge Empfindlichkeit anpassen." } },
    { ELanguage::French,   { "Sensibilité du Stick", "Ajuster la sensibilité analogique." } },
    { ELanguage::Spanish,  { "Sensibilidad del stick", "Ajusta la sensibilidad del stick." } },
    { ELanguage::Italian,  { "Sensibilità dello Stick", "Regola la sensibilità dello stick analogico." } }
};

// shared by all players
CONFIG_LOCALE g_StickSensitivity2P_locale = g_StickSensitivity1P_locale;
CONFIG_LOCALE g_StickSensitivity3P_locale = g_StickSensitivity1P_locale;
CONFIG_LOCALE g_StickSensitivity4P_locale = g_StickSensitivity1P_locale;

CONFIG_DEFINE_LOCALE(MasterVolume)
{
    { ELanguage::English, { "Master Volume", "Adjust the overall volume." } },
    { ELanguage::Japanese, { "マスターボリューム", "ゲーム全体の音量を変更します。" } },
    { ELanguage::German,   { "Gesamtlautstärke", "Passe die Gesamtlautstärke an." } },
    { ELanguage::French,   { "Volume général", "Réglage du volume général." } },
    { ELanguage::Spanish,  { "Volumen maestro", "Ajusta el volumen general." } },
    { ELanguage::Italian,  { "Volume principale", "Regola il volume principale" } }
};

CONFIG_DEFINE_LOCALE(MusicVolume)
{
    { ELanguage::English,  { "Music Volume", "Adjust the volume for music." } },
    { ELanguage::Japanese, { "BGM", "BGMの音量を変更します。" } },
    { ELanguage::German,   { "Musiklautstärke", "Passe die Lautstärke der Musik an." } },
    { ELanguage::French,   { "Volume de la musique", "Réglage du volume de la musique." } },
    { ELanguage::Spanish,  { "Volumen de la música", "Ajusta el volumen de la música." } },
    { ELanguage::Italian,  { "Volume musica di sottofondo", "Regola il volume della musica di sottofondo." } }
};

CONFIG_DEFINE_LOCALE(EffectsVolume)
{
    { ELanguage::English,  { "Effects Volume", "Adjust the volume for sound effects." } },
    { ELanguage::Japanese, { "SE", "SEの音量を変更します。" } },
    { ELanguage::German,   { "Soundeffektlautstärke", "Passe die Lautstärke der Soundeffekte an." } },
    { ELanguage::French,   { "Volume des effets sonores", "Réglage du volume des effets sonores." } },
    { ELanguage::Spanish,  { "Volumen de los efectos", "Ajusta el volumen de los efectos de sonido." } },
    { ELanguage::Italian,  { "Volume effetti sonori", "Regola il volume degli effetti sonori." } }
};

CONFIG_DEFINE_LOCALE(VoicesVolume)
{
    { ELanguage::English,  { "Voices Volume", "Adjust the volume for voices." } },
    { ELanguage::Japanese, { "ボイス", "キャラクターボイスの音量を変更します。" } },
    { ELanguage::German,   { "Stimmenlautstärke", "Passe die Lautstärke der Stimmen an." } },
    { ELanguage::French,   { "Volume des voix", "Réglage du volume des voix." } },
    { ELanguage::Spanish,  { "Volumen de las voces", "Ajusta el volumen de las voces." } },
    { ELanguage::Italian,  { "Volume delle voci", "Regola il volume delle voci." } },
};

CONFIG_DEFINE_LOCALE(CutsceneVolume)
{
    { ELanguage::English,  { "Cutscene Volume", "Adjust the volume for cutscenes." } },
    { ELanguage::Japanese, { "イベント", "イベントシーンの音量を変更します。" } },
    { ELanguage::German,   { "Zwischensequenz-lautstärke", "Passe die Lautstärke der Zwischensequenzen an." } },
    { ELanguage::French,   { "Volume des cinématiques", "Réglage du volume des cinématiques." } },
    { ELanguage::Spanish,  { "Volumen de las cinemáticas", "Ajusta el volumen de las cinemáticas." } },
    { ELanguage::Italian,  { "Volume dei filmati", "Regola il volume per i filmati" } }
};


CONFIG_DEFINE_LOCALE(MusicAttenuation)
{
    { ELanguage::English,  { "Music Attenuation", "Fade out the game's music when external media is playing." } },
    { ELanguage::Japanese, { "BGM減衰", "外部メディアの再生中、ゲームの音楽をフェードアウトします。" } },
    { ELanguage::German,   { "Musikdämpfung", "Stelle die Musik des Spiels stumm während externe Medien abgespielt werden." } },
    { ELanguage::French,   { "Atténuation audio", "Abaisse le volume des musiques du jeu lorsqu'un média externe est en cours de lecture." } },
    { ELanguage::Spanish,  { "Atenuación de la música", "Atenúa la música del juego cuando un reproductor multimedia se encuentre activo." } },
    { ELanguage::Italian,  { "Attenuazione musica", "Abbassa il volume della musica di sottofondo quando un'altra applicazione riproduce dei suoni." } }
};

CONFIG_DEFINE_LOCALE(MuteOnFocusLost)
{
    { ELanguage::English,  { "Mute on Focus Lost", "Mute the game's audio when the window is not in focus." } },
    { ELanguage::Japanese, { "非アクティブミュート", "ウィンドウが非アクティブ時に、ゲームの音声をミュートします。" } },
    { ELanguage::German,   { "Stummstellen wenn nicht fokussiert", "Stellt das Audio des Spiels stumm solange das Fenster nicht im Fokus ist." } },
    { ELanguage::French,   { "Muet si fenêtre inactive", "Coupe le son du jeu lorsque la fenêtre n'est pas active." } },
    { ELanguage::Spanish,  { "Silenciar en segundo plano", "Silencia el audio del juego cuando la ventana no esté en primer plano." } },
    { ELanguage::Italian,  { "Audio muto con finestra inattiva", "Disattiva l'audio del gioco quando la finestra non è attiva." } }
};


CONFIG_DEFINE_LOCALE(IdleChaosPowersMusic)
{
    { ELanguage::English,  { "Chaos Power Music", "Play the theme when either chaos gauge is full. 'OFF' will continue to play the stage music instead." } },
    { ELanguage::Japanese, { "カオスパワーBGM", "ヒーローゲージまたはダークゲージが満タンになるとBGMを再生します。「OFF」にすると、代わりにステージBGMが流れ続けます。" } },
    { ELanguage::German,   { "Chaos Power Musik", "Spiele die Musik ab, wenn eine der Chaosanzeigen voll ist. Bei der Einstellung „AUS“ wird stattdessen die Levelmusik weiter abgespielt." } },
    { ELanguage::French,   { "Musique des Chaos Power", "Joue le thème lorsque l'un des indicateurs de chaos est plein. 'NON' continuera à jouer la musique du niveau." } },
    { ELanguage::Spanish,  { "Música de los Chaos Power", "Reproduce el tema cuando cualquier indicador chaos está lleno. Si seleccionas 'NO', la música del nivel continuará sonando." } },
    { ELanguage::Italian,  { "Musica Chaos Power", "La musica parte quando entrambi i Chaos gauge sono pieni. 'OFF' farà continuare invece la musica del quadro." } }
};


CONFIG_DEFINE_LOCALE(ChaosPowerLEDSync)
{
    { ELanguage::English,  { "Chaos Power LED Sync", "Sync controller LED color with the active Chaos Power." } },
    { ELanguage::Japanese, { "カオスパワーLED同期", "コントローラーのLEDの色を、発動中のカオスパワーに合わせます。" } },
    { ELanguage::German,   { "Chaos Power LED Sync", "Synchronisiere die LED-Farbe des Controllers mit der aktiven Chaos Power." } },
    { ELanguage::French,   { "Synchronisation LED Chaos Power", "Synchronise la couleur des LED de la manette avec le Chaos Power actif." } },
    { ELanguage::Spanish,  { "Sincronización LED de los Chaos Power", "Sincroniza el LED del mando con el color del Chaos Power activo." } },
    { ELanguage::Italian,  { "Sincronizzazione LED Chaos Power", "Sincronizza il colore del LED quando Chaos Power è attivo." } }
};

CONFIG_DEFINE_LOCALE(ControllerLEDBrightness)
{
    { ELanguage::English,  { "Controller LED Brightness", "LED brightness for compatible controllers." } },
    { ELanguage::Japanese, { "コントローラーLEDの明るさ", "対応コントローラーのLEDの明るさを調整します。" } },
    { ELanguage::German,   { "Controller LED Brightness", "LED-Helligkeit für kompatible Controller." } },
    { ELanguage::French,   { "Luminosité LED du contrôleur", "Luminosité des LED pour les contrôleurs compatibles." } },
    { ELanguage::Spanish,  { "Brillo LED del mando", "Brillo del LED para mandos compatibles." } },
    { ELanguage::Italian,  { "Luminosità LED Controller", "Luminosità del LED per i controller compatibili." } }
};


CONFIG_DEFINE_LOCALE(VoiceLanguage)
{
    { ELanguage::English, { "Voice Language", "Change the spoken language." } },
    { ELanguage::Japanese, { "音声言語", "音声で流れてくる言語を変更します。" } },
    { ELanguage::German,  { "Sprachausgabe ", "Ändere die gesprochene Sprache." } },
    { ELanguage::French,   { "Langue des Voix", "Changez la langue des voix du jeu." } },
    { ELanguage::Spanish,  { "Idioma de las voces", "Cambia el idioma de las voces del juego." } },
    { ELanguage::Italian, { "Linguaggio voci", "Cambia la lingua parlata nei dialoghi." } },
};

CONFIG_DEFINE_LOCALE(Subtitles)
{
    { ELanguage::English,  { "Subtitles", "Show subtitles during dialogue." } },
    { ELanguage::Japanese, { "字幕", "字幕の表示を選択できます。" } },
    { ELanguage::German,   { "Untertitel", "Zeige Untertitel bei Dialogen." } },
    { ELanguage::French,   { "Sous-titres", "Affiche les sous-titres pendant les dialogues." } },
    { ELanguage::Spanish,  { "Subtítulos", "Selecciona la configuración de los subtítulos." } },
    { ELanguage::Italian,  { "Sottotitoli", "Mostra i sottotitoli durante i dialoghi." } }
};

CONFIG_DEFINE_LOCALE(GraphicsAPI)
{
    { ELanguage::English,  { "Rendering Backend", "Set graphics API used for rendering. Only change this if you experience visual issues." } },
    { ELanguage::Japanese, { "レンダリングバックエンド", "レンダリングに使用するグラフィックAPIを設定します。映像に問題が発生した場合のみ変更してください。" } },
    { ELanguage::German,   { "Rendering-Backend", "Legt die für das Rendering verwendete Grafik-API fest. Ändere diese Einstellung nur, wenn visuelle Probleme auftreten." } },
    { ELanguage::French,   { "Moteur de Rendu", "Sélectionnez le moteur graphique. Ne modifiez ce paramètre que si vous rencontrez des problèmes visuels dans le jeu." } },
    { ELanguage::Spanish,  { "Motor de gráficos", "Selecciona el motor de gráficos. Solamente cambia esto si encuentras problemas visuales en el juego." } },
    { ELanguage::Italian,  { "Rendering Backend", "Imposta L'API Grafico usato per il rendering. Cambia questa opzione solamente se stai sperimentando problemi visivi." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EGraphicsAPI)
{
    {
        ELanguage::English,
        {
            { EGraphicsAPI::Auto, { "AUTO", "Render using recommended backend." } },
#ifdef HEROES_RECOMP_D3D12
            { EGraphicsAPI::D3D12, { "D3D12", "Render using Direct3D 12." } },
#endif
            { EGraphicsAPI::Vulkan, { "VULKAN", "Render using Vulkan." } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { EGraphicsAPI::Auto, { "自動", "推奨バックエンドでレンダリングします。" } },
#ifdef HEROES_RECOMP_D3D12
            { EGraphicsAPI::D3D12, { "D3D12", "Direct3D 12でレンダリングします。" } },
#endif
            { EGraphicsAPI::Vulkan, { "VULKAN", "Vulkanでレンダリングします。" } }
        }
    },
    {
        ELanguage::German,
        {
            { EGraphicsAPI::Auto, { "AUTO", "Rendern mit dem empfohlenen Grafik-Backend." } },
#ifdef HEROES_RECOMP_D3D12
            { EGraphicsAPI::D3D12, { "D3D12", "Rendern mit Direct3D 12." } },
#endif
            { EGraphicsAPI::Vulkan, { "VULKAN", "Rendern mit Vulkan." } }
        }
    },
    {
        ELanguage::French,
        {
            { EGraphicsAPI::Auto, { "AUTO", "Utiliser le moteur recommandé." } },
#ifdef HEROES_RECOMP_D3D12
            { EGraphicsAPI::D3D12, { "D3D12", "Utiliser Direct3D 12." } },
#endif
            { EGraphicsAPI::Vulkan, { "VULKAN", "Utiliser Vulkan." } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { EGraphicsAPI::Auto, { "AUTO", "Utiliza el motor recomendado." } },
#ifdef HEROES_RECOMP_D3D12
            { EGraphicsAPI::D3D12, { "D3D12", "Utiliza Direct3D 12." } },
#endif
            { EGraphicsAPI::Vulkan, { "VULKAN", "Utiliza Vulkan." } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EGraphicsAPI::Auto, { "AUTO", "Usa il backend raccomandato per il rendering." } },
#ifdef HEROES_RECOMP_D3D12
            { EGraphicsAPI::D3D12, { "D3D12", "Usa Direct3D 12 per il rendering." } },
#endif
            { EGraphicsAPI::Vulkan, { "VULKAN", "Usa Vulkan per il rendering." } }
        }
    }
};

CONFIG_DEFINE_LOCALE(WindowSize)
{
    { ELanguage::English,  { "Resolution", "Adjust the resolution of the game in windowed mode." } },
    { ELanguage::Japanese, { "解像度", "ウィンドウモードでのゲームの解像度を調整できます。" } },
    { ELanguage::German,   { "Auflösung", "Ändere die Auflösung des Spiels im Fenstermodus." } },
    { ELanguage::French,   { "Résolution", "Modifie la résolution du jeu en mode fenêtré." } },
    { ELanguage::Spanish,  { "Resolución", "Ajusta la resolución del juego en modo ventana." } },
    { ELanguage::Italian,  { "Risoluzione", "Regola la risoluzione del gioco in modalità finestra." } }
};

CONFIG_DEFINE_LOCALE(Monitor)
{
    { ELanguage::English,  { "Monitor", "Change which monitor to display the game on." } },
    { ELanguage::Japanese, { "モニター選択", "ゲームを表示するモニターを変更できます。" } },
    { ELanguage::German,   { "Monitor", "Ändere auf welchem Monitor das Spiel angezeigt wird." } },
    { ELanguage::French,   { "Moniteur", "Change le moniteur sur lequel le jeu sera affiché." } },
    { ELanguage::Spanish,  { "Monitor", "Selecciona el monitor de visualización." } },
    { ELanguage::Italian,  { "Schermo", "Cambia lo schermo su cui visualizzare il gioco." } }
};

CONFIG_DEFINE_LOCALE(AspectRatio)
{
    { ELanguage::English,  { "Aspect Ratio", "Change the aspect ratio." } },
    { ELanguage::Japanese, { "アスペクト比", "アスペクト比を変更できます。" } },
    { ELanguage::German,   { "Seitenverhältnis", "Verändere das Seitenverhältnis." } },
    { ELanguage::French,   { "Format d'Affichage", "Modifie le format d'affichage." } },
    { ELanguage::Spanish,  { "Relación de aspecto", "Selecciona la relación de aspecto." } },
    { ELanguage::Italian,  { "Rapporto d'aspetto", "Modifica il rapporto d'aspetto." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EAspectRatio)
{
    {
        ELanguage::English,
        {
            { EAspectRatio::Auto, { "AUTO", "Dynamically adjusts the aspect ratio to the window size." } },
            { EAspectRatio::Original, { "4:3 (Original)", "The game's original aspect ratio." } },
            { EAspectRatio::Wide16x9, { "16:9", "Force a 16:9 aspect ratio." } },
            { EAspectRatio::Wide16x10, { "16:10", "Force a 16:10 aspect ratio." } },
            { EAspectRatio::Narrow5x4, { "5:4", "Force a 5:4 aspect ratio." } },
            { EAspectRatio::Wide21x9, { "21:9", "Force a 21:9 aspect ratio." } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { EAspectRatio::Auto, { "自動", "ウィンドウサイズに合わせてアスペクト比を動的に調整します。" } },
            { EAspectRatio::Original, { "4:3 (Original)", "ゲーム本来のアスペクト比にします。" } },
            { EAspectRatio::Wide16x9, { "16:9", "アスペクト比を16:9に固定します。" } },
            { EAspectRatio::Wide16x10, { "16:10", "アスペクト比を16:10に固定します。" } },
            { EAspectRatio::Narrow5x4, { "5:4", "アスペクト比を5:4に固定します。" } },
            { EAspectRatio::Wide21x9, { "21:9", "アスペクト比を21:9に固定します。" } }
        }
    },
    {
        ELanguage::German,
        {
            { EAspectRatio::Auto, { "AUTO", "Passt das Seitenverhältnis dynamisch an die Fenstergröße an" } },
            { EAspectRatio::Original, { "4:3 (Original)", "Das originale Seitenverhältnis des Spiels" } },
            { EAspectRatio::Wide16x9, { "16:9", "Erzwinge ein Seitenverhältnis von 16:9" } },
            { EAspectRatio::Wide16x10, { "16:10", "Erzwinge ein Seitenverhältnis von 16:10" } },
            { EAspectRatio::Narrow5x4, { "5:4", "Erzwinge ein Seitenverhältnis von 5:4" } },
            { EAspectRatio::Wide21x9, { "21:9", "Erzwinge ein Seitenverhältnis von 21:9" } }
        }
    },
    {
        ELanguage::French,
        {
            { EAspectRatio::Auto, { "AUTO", "Ajuste dynamiquement le format d'affichage en fonction de la taille de la fenêtre." } },
            { EAspectRatio::Original, { "4:3 (Original)", "Le format d'affichage du jeu de base." } },
            { EAspectRatio::Wide16x9, { "16:9", "Force le format d'affichage en 16:9." } },
            { EAspectRatio::Wide16x10, { "16:10", "Force le format d'affichage en 16:10." } },
            { EAspectRatio::Narrow5x4, { "5:4", "Force le format d'affichage en 5:4." } },
            { EAspectRatio::Wide21x9, { "21:9", "Force le format d'affichage en 21:9." } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { EAspectRatio::Auto, { "AUTO", "Ajusta de forma dinámica la relación de aspecto para ajustarse a la pantalla." } },
            { EAspectRatio::Original, { "4:3 (Original)", "La relación de aspecto del juego original." } },
            { EAspectRatio::Wide16x9, { "16:9", "Forzar 16:9" } },
            { EAspectRatio::Wide16x10, { "16:10", "Forzar 16:10" } },
            { EAspectRatio::Narrow5x4, { "5:4", "Forzar 5:4" } },
            { EAspectRatio::Wide21x9, { "21:9", "Forzar 21:9" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EAspectRatio::Auto, { "AUTO", "Sistema in modo dinamico il rapporto d'aspetto in base alla grandezza della finestra." } },
            { EAspectRatio::Original, { "4:3 (Originale)", "Il rapporto d'aspetto del gioco originale." } },
            { EAspectRatio::Wide16x9, { "16:9", "Forza il rapporto d'aspetto 16:9." } },
            { EAspectRatio::Wide16x10, { "16:10", "Forza il rapporto d'aspetto 16:10." } },
            { EAspectRatio::Narrow5x4, { "5:4", "Forza il rapporto d'aspetto 5:4." } },
            { EAspectRatio::Wide21x9, { "21:9", "Forza il rapporto d'aspetto 21:9." } }
        }
    }
};

CONFIG_DEFINE_LOCALE(FMVAspectRatio)
{
    { ELanguage::English,  { "FMV Aspect Ratio", "Change how full motion videos (pre-rendered cutscenes) fit the display." } },
    { ELanguage::Japanese, { "FMVのアスペクト比", "フルモーションビデオ（プリレンダリングされたイベントシーン）の画面への表示方法を変更します。" } },
    { ELanguage::German,   { "FMV-Seitenverhältnis", "Ändere wie vorgerenderte Zwischensequenzen an das Display angepasst werden." } },
    { ELanguage::French,   { "Format d'Affichage des vidéos Pré-rendu", "Sélectionnez le format d'image des vidéos pré-rendues." } },
    { ELanguage::Spanish,  { "Relación de aspecto de los vídeos pre-renderizados", "Selecciona la relación de aspecto de los vídeos pre-renderizados." } },
    { ELanguage::Italian,  { "Rapporto d'aspetto FMV", "Cambia come i full motion videos (Filmati pre-registrati) si adatti al display." } }
};


CONFIG_DEFINE_ENUM_LOCALE(EFMVAspectRatio)
{
    {
        ELanguage::English,
        {
            { EFMVAspectRatio::Stretched, { "STRETCHED", "The video is stretched to fill the display." } },
            { EFMVAspectRatio::Fit,       { "FIT", "The video keeps its original aspect ratio with black borders." } },
            { EFMVAspectRatio::Fill,      { "FILL", "The video is cropped and zoomed in with its original aspect ratio." } },
        }
    },
    {
        ELanguage::Japanese,
        {
            { EFMVAspectRatio::Stretched, { "引き伸ばし", "映像を画面いっぱいに引き伸ばします。" } },
            { EFMVAspectRatio::Fit,       { "フィット", "映像の元のアスペクト比を保ち、周囲に黒い枠が表示されます。" } },
            { EFMVAspectRatio::Fill,      { "画面いっぱい", "映像の元のアスペクト比を保ったまま、トリミングされて拡大表示されます。" } },
        }
    },
    {
        ELanguage::German,
        {
            { EFMVAspectRatio::Stretched, { "GESTRECKT", "Das Video wird gestreckt, um den Bildschirm auszufüllen." } },
            { EFMVAspectRatio::Fit,       { "PASSEN", "Das Video behält das originale Seitenverhältnis mit schwarzen Rändern bei." } },
            { EFMVAspectRatio::Fill,      { "FÜLLEN", "Das Video wird beschnitten und dann vergrößert." } },
        }
    },
    {
        ELanguage::French,
        {
            { EFMVAspectRatio::Stretched, { "ÉTIRÉE", "La vidéo est étirée pour remplir l'écran." } },
            { EFMVAspectRatio::Fit,       { "AJUSTER", "La vidéo conserve son ratio d'aspect original avec des bordures noires." } },
            { EFMVAspectRatio::Fill,      { "REMPLIR", "La vidéo est rognée et zoomée avec son ratio d'aspect original." } },
        }
    },
    {
        ELanguage::Spanish,
        {
            { EFMVAspectRatio::Stretched, { "ESTIRAR", "El vídeo se estirará para ocupar la pantalla." } },
            { EFMVAspectRatio::Fit,       { "ADAPTAR", "El vídeo mantendrá su relación de aspecto original, añadiendo bordes negros." } },
            { EFMVAspectRatio::Fill,      { "RELLENAR", "El vídeo se recortará y ampliará, manteniendo su relación de aspecto original." } },
        }
    },
    {
        ELanguage::Italian,
        {
            { EFMVAspectRatio::Stretched, { "ALLUNGATO", "Il video viene allungato per riempire il display." } },
            { EFMVAspectRatio::Fit,       { "ADATTA", "Il video mantiene il rapporto d'aspetto originale con bordi neri." } },
            { EFMVAspectRatio::Fill,      { "RIEMPI", "Il video viene tagliato e ingrandito con il suo rapporto d'aspetto originale." } },
        }
    }
};

CONFIG_DEFINE_LOCALE(ResolutionScale)
{
    { ELanguage::English,  { "Resolution Scale", "Adjust the internal resolution of the game." } },
    { ELanguage::Japanese, { "解像度スケール", "ゲームの内部解像度を調整できます。" } },
    { ELanguage::German,   { "Rendering-Auflösung", "Passe die Auflösung der internen Auflösung an." } },
    { ELanguage::French,   { "Échelle de rendu", "Modifie la résolution interne du jeu." } },
    { ELanguage::Spanish,  { "Escala de resolución", "Selecciona la resolución interna del juego." } },
    { ELanguage::Italian,  { "Scala risoluzione", "Regola la risoluzione interna del gioco." } }
};

CONFIG_DEFINE_LOCALE(Fullscreen)
{
    { ELanguage::English,  { "Fullscreen", "Toggle between borderless fullscreen or windowed mode." } },
    { ELanguage::Japanese, { "フルスクリーン", "ボーダーレスフルスクリーンかウィンドウモードを選択できます。" } },
    { ELanguage::German,   { "Vollbild", "Wechsle zwischen dem randlosen Vollbildmodus und dem Fenstermodus." } },
    { ELanguage::French,   { "Plein écran", "Alterne entre le mode plein écran sans bordures et le mode fenêtré." } },
    { ELanguage::Spanish,  { "Pantalla completa", "Cambia entre modo de pantalla completa o ventana." } },
    { ELanguage::Italian,  { "Schermo pieno", "Attiva/disattiva tra modalità finestra senza cornice e modalità finestra." } }
};

CONFIG_DEFINE_LOCALE(VSync)
{
    { ELanguage::English,  { "V-Sync", "Synchronize the game to the refresh rate of the display to prevent screen tearing." } },
    { ELanguage::Japanese, { "垂直同期", "垂直同期の設定を変更できます。" } },
    { ELanguage::German,   { "V-Sync", "Synchronisiere das Spiel mit der Bildwiederholrate deines Bildschirms um Bildverzerrungen zu vermeiden." } },
    { ELanguage::French,   { "V-Sync", "Activez la synchronisation verticale pour éviter les déchirures de l'image." } },
    { ELanguage::Spanish,  { "V-Sync", "Activa la sincronización vertical para prevenir el rasgado de la imagen." } },
    { ELanguage::Italian,  { "V-Sync", "Sincronizza il gioco con la frequenza d'aggiornamento del display per evitare lo screen tearing." } }
};

CONFIG_DEFINE_LOCALE(FPS)
{
    { ELanguage::English,  { "FPS", "Set the max frame rate the game can run at. WARNING: this may introduce glitches at frame rates higher than 60 FPS." } },
    { ELanguage::Japanese, { "フレームレート上限", "ゲームの最大フレームレートを設定できます。警告：60 FPSを超えるフレームレートで不具合が発生する可能性があります。" } },
    { ELanguage::German,   { "FPS", "Setze die maximale Anzahl der Bilder pro Sekunde, die das Spiel darstellen kann. WARNUNG: Das Spiel kann bei höheren FPS als 60 ungewolltes Verhalten aufweisen." } },
    { ELanguage::French,   { "IPS", "Détermine la fréquence d'images maximale du jeu. ATTENTION : cela peut entraîner des problèmes à des taux de rafraîchissement supérieurs à 60 IPS." } },
    { ELanguage::Spanish,  { "FPS", "Selecciona el límite de fotogramas por segundo. ADVERTENCIA: Límites por encima de los 60 FPS pueden introducir fallos en el juego." } },
    { ELanguage::Italian,  { "FPS", "Imposta il frame rate massimo del gioco. ATTENZIONE: questa opzione può causare dei glitch a frame rate più alti di 60 FPS." } }
};

CONFIG_DEFINE_LOCALE(CutsceneFramerate)
{
    { ELanguage::English,  { "Cutscene Framerate", "Set the frame rate of in-game cutscenes." } },
    { ELanguage::Japanese, { "イベントシーンのフレームレート", "ゲーム内イベントシーンのフレームレートを設定します。" } },
    { ELanguage::German,   { "Zwischensequenz-Framerate", "Lege fest die Bildrate der Zwischensequenzen." } },
    { ELanguage::French,   { "IPS des cinématiques", "Détermine la fréquence d'images des cinématiques en jeu." } },
    { ELanguage::Spanish,  { "FPS en cinemáticas", "Selecciona el límite de fotogramas por segundo en las cinemáticas (no se aplica en los vídeos pre-renderizados)." } },
    { ELanguage::Italian,  { "Framerate filmati", "Imposta il frame rate per i filmati in-game." } }
};

CONFIG_DEFINE_ENUM_LOCALE(ECutsceneFramerate)
{
    {
        ELanguage::English,
        {
            { ECutsceneFramerate::Original, { "ORIGINAL", "In-game cutscenes run at 30 FPS." } },
            { ECutsceneFramerate::Unlocked, { "UNLOCKED", "In-game cutscenes run at target framerate." } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { ECutsceneFramerate::Original, { "オリジナル", "ゲーム内イベントシーンを30 FPSで再生します。" } },
            { ECutsceneFramerate::Unlocked, { "制限解除", "ゲーム内イベントシーンを目標フレームレートで再生します。" } }
        }
    },
    {
        ELanguage::German,
        {
            { ECutsceneFramerate::Original, { "ORIGINAL", "Die Zwischensequenzen laufen auf 30 FPS." } },
            { ECutsceneFramerate::Unlocked, { "UNBEGRENZT", "Die Zwischensequenzen laufen mit der Zielbildrate." } }
        }
    },
    {
        ELanguage::French,
        {
            { ECutsceneFramerate::Original, { "ORIGINAL", "Les cinématiques en jeu tournent à 30 IPS." } },
            { ECutsceneFramerate::Unlocked, { "DÉVERROUILLÉ", "Les cinématiques en jeu tournent à la limite du nombre d'images." } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { ECutsceneFramerate::Original, { "ORIGINAL", "Las cinemáticas funcionan a 30 FPS." } },
            { ECutsceneFramerate::Unlocked, { "DESBLOQUEADO", "Las cinemáticas funcionan al límite de fotogramas." } }
        }
    },
    {
        ELanguage::Italian,
        {
            { ECutsceneFramerate::Original, { "ORIGINALE", "In-game cutscenes run at 30 FPS." } },
            { ECutsceneFramerate::Unlocked, { "SBLOCCATO", "Filmati In-game gireranno al framerate impostato." } }
        }
    }
};

CONFIG_DEFINE_LOCALE(ShowFPS)
{
    { ELanguage::English,  { "Show FPS", "Shows the framerate counter." } },
    { ELanguage::Japanese, { "FPSを表示", "フレームレートを表示します。" } },
    { ELanguage::German,   { "Zeige FPS", "Zeigt den Framerate-Counter an." } },
    { ELanguage::French,   { "Montrer les IPS", "Affiche le compteur de fréquence d'images." } },
    { ELanguage::Spanish,  { "Mostrar FPS", "Muestra el contador de FPS." } },
    { ELanguage::Italian,  { "Mostra FPS", "Mostra il contatore framerate." } }
};

CONFIG_DEFINE_LOCALE(Brightness)
{
    { ELanguage::English, { "Brightness", "Adjust the brightness level." } },
    { ELanguage::Japanese, { "明るさ設定", "画面の明るさを調整します。" } },
    { ELanguage::German,   { "Helligkeit", "Passe die Helligkeit an." } },
    { ELanguage::French,   { "Luminosité", "Réglez le niveau de luminosité." } },
    { ELanguage::Spanish,  { "Brillo", "Ajusta el nivel del brillo." } },
    { ELanguage::Italian,  { "Luminosità", "Regola il livello di luminosità." } }
};

CONFIG_DEFINE_LOCALE(BloomIntensity)
{
    { ELanguage::English, { "Bloom Intensity", "Adjust the intensity of the bloom effect." } },
    { ELanguage::Japanese, { "ブルーム強度", "ブルーム効果の強さを調整します。" } },
    { ELanguage::German,   { "Bloom-Intensität", "Passe die Intensität des Bloom-Effekts an." } },
    { ELanguage::French,   { "Intensité du Bloom", "Réglez l'intensité de l'effet du bloom." } },
    { ELanguage::Spanish,  { "Intensidad del bloom", "Ajusta la intensidad de las fuentes de luz." } },
    { ELanguage::Italian,  { "Intensità del bloom", "Regola l'intensità dell'effetto bloom." } }
};


CONFIG_DEFINE_LOCALE(EnablePartnerIntro)
{
    { ELanguage::English,  { "Partner Intro", "Show intro when meeting new partners." } },
    { ELanguage::Japanese, { "パートナー紹介", "新しいパートナーキャラクターと出会ったときに紹介演出を表示します。" } },
    { ELanguage::German,   { "Partner Intro", "Zeige Intro, wenn Sie einen neuen Partner kennenlernen." } },
    { ELanguage::French,   { "Intro du Partenaire", "Afficher l'introduction du partenaire lorsqu'il est rencontré pour la première fois." } },
    { ELanguage::Spanish,  { "Introducción del compañero", "Muestra la introducción al encontrarse con un compañero nuevo." } },
    { ELanguage::Italian,  { "Introduzione Partner", "Mostra un intro quando incontri nuovi partners." } }
};

CONFIG_DEFINE_LOCALE(MaxHUDWidth)
{
    { ELanguage::English,  { "HUD Width", "Constrain HUD to a chosen aspect ratio. Primarily for ultrawide monitors." } },
    { ELanguage::Japanese, { "HUDの幅", "HUDの幅を指定したアスペクト比に合わせます。主にウルトラワイドモニター向けの設定です。" } },
    { ELanguage::German,   { "HUD Breite", "Beschränken Sie das HUD auf ein ausgewähltes Seitenverhältnis. Hauptsächlich für Ultrawide-Monitore." } },
    { ELanguage::French,   { "Largeur de l'interface", "Limite la largeur de l'interface à un ratio d'aspect choisi. Principalement conçue pour les moniteurs ultra-larges." } },
    { ELanguage::Spanish,  { "Ancho de la interfaz", "Limita la interfaz a la relación de aspecto seleccionada. Esta opción es principalmente para monitores ultra anchos." } },
    { ELanguage::Italian,  { "Regolazione HUD", "Forza L'HUD al rapporto d'aspetto impostato. Usato primariamente per monitor ultrawide." } }
};
