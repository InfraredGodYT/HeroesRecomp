#include <user/config.h>
#include <locale/locale.h>

std::unordered_map<std::string_view, std::unordered_map<ELanguage, std::string>> g_locale =
{
	{
		"Options_Category_System",
		{
			{ ELanguage::English,  "SYSTEM" },
			{ ELanguage::Japanese, "システム" },
			{ ELanguage::German,   "SYSTEM" },
			{ ELanguage::French,   "SYSTÈME" },
			{ ELanguage::Spanish,  "SISTEMA" },
			{ ELanguage::Italian,  "SISTEMA" }
		}
	},
	{
		"Options_Desc_Category_System",
		{
			{ ELanguage::English,  "Adjust system settings." },
			{ ELanguage::Japanese, "システムの設定を変更します。" },
			{ ELanguage::German,   "Verändere System-Einstellungen." },
			{ ELanguage::French,   "Modifier les paramètres système." },
			{ ELanguage::Spanish,  "Ajusta la configuración de sistema." },
			{ ELanguage::Italian,  "Modifica le impostazioni di sistema." }
		}
	},
	{
		"Options_Category_Input",
		{
			{ ELanguage::English,  "INPUT" },
			{ ELanguage::Japanese, "操作" },
			{ ELanguage::German,   "EINGABE" },
			{ ELanguage::French,   "COMMANDES" },
			{ ELanguage::Spanish,  "CONTROLES" },
			{ ELanguage::Italian,  "COMANDI" }
		}
	},
	{
		"Options_Desc_Category_Input",
		{
			{ ELanguage::English,  "Adjust input settings." },
			{ ELanguage::Japanese, "操作の設定を変更します。" },
			{ ELanguage::German,   "Verändere Eingabe-Einstellungen." },
			{ ELanguage::French,   "Modifier les paramètres des commandes." },
			{ ELanguage::Spanish,  "Ajusta la configuración de entrada." },
			{ ELanguage::Italian,  "Modifica le impostazioni input." }
		}
	},
	{
		"Options_Category_Audio",
		{
			{ ELanguage::English,  "AUDIO" },
			{ ELanguage::Japanese, "オーディオ" },
			{ ELanguage::German,   "AUDIO" },
			{ ELanguage::French,   "AUDIO" },
			{ ELanguage::Spanish,  "AUDIO" },
			{ ELanguage::Italian,  "AUDIO" }
		}
	},
	{
		"Options_Desc_Category_Audio",
		{
			{ ELanguage::English,  "Adjust audio settings." },
			{ ELanguage::Japanese, "オーディオの設定を変更します。" },
			{ ELanguage::German,   "Verändere Audio-Einstellungen." },
			{ ELanguage::French,   "Modifier les paramètres audio." },
			{ ELanguage::Spanish,  "Ajusta la configuración de audio." },
			{ ELanguage::Italian,  "Modifica le impostazioni audio." }
		}
	},
	{
		"Options_Category_Video",
		{
			{ ELanguage::English,  "VIDEO" },
			{ ELanguage::Japanese, "グラフィック" },
			{ ELanguage::German,   "VIDEO" },
			{ ELanguage::French,   "VIDÉO" },
			{ ELanguage::Spanish,  "VÍDEO" },
			{ ELanguage::Italian,  "VIDEO" }
		}
	},
	{
		"Options_Desc_Category_Video",
		{
			{ ELanguage::English,  "Adjust video settings." },
			{ ELanguage::Japanese, "グラフィックの設定を変更します。" },
			{ ELanguage::German,   "Verändere Video-Einstellungen." },
			{ ELanguage::French,   "Modifier les paramètres vidéo." },
			{ ELanguage::Spanish,  "Ajusta la configuración de vídeo." },
			{ ELanguage::Italian,  "Modifica le impostazioni video." }
		}
	},
	{
		// Notes: integer values in the options menu (e.g. FPS) when at their maximum value.
		"Options_Value_Max",
		{
			{ ELanguage::English,  "MAX" },
			{ ELanguage::Japanese, "最大" },
			{ ELanguage::German,   "MAX" },
			{ ELanguage::French,   "MAX" },
			{ ELanguage::Spanish,  "MÁX" },
			{ ELanguage::Italian,  "MAX" }
		}
	},
	{
		"Options_Value_Custom",
		{
			{ ELanguage::English,  "CUSTOM" },
			{ ELanguage::Japanese, "カスタム" },
			{ ELanguage::German,   "BENUTZERDEFINIERT" },
			{ ELanguage::French,   "CUSTOM" },
			{ ELanguage::Spanish,  "PERSONALIZADO" },
			{ ELanguage::Italian,  "CUSTOM" }
		}
	},
	{
		"Options_Value_AllScreens",
		{
			{ ELanguage::English,  "ALL SCREENS" },
			{ ELanguage::Japanese, "すべての画面" },
			{ ELanguage::German,   "ALLE BILDSCHIRME" },
			{ ELanguage::French,   "TOUS LES ÉCRANS" },
			{ ELanguage::Spanish,  "TODAS LAS PANTALLAS" },
			{ ELanguage::Italian,  "ALL SCREENS" },
		}
	},
   {
	   // Notes: description for Music Attenuation when the user is not running a supported OS.
	   "Options_Desc_OSNotSupported",
	   {
		   { ELanguage::English,  "This option is not supported by your operating system." },
		   { ELanguage::Japanese, "このオプションは、お使いのOSではサポートされていません。" },
		   { ELanguage::German,   "Diese Option wird von deinem Betriebssystem nicht unterstützt." },
		   { ELanguage::French,   "Cette option n'est pas prise en charge par votre système d'exploitation." },
		   { ELanguage::Spanish,  "Esta opción no es compatible con tu sistema operativo." },
		   { ELanguage::Italian,  "Questa opzione non è disponibile con il tuo sistema operativo." }
	   }
},
{
	"Options_Name_CustomizeInput",
	{
		{ ELanguage::English,  "Customize Input" },
		{ ELanguage::Japanese, "操作設定" },
		{ ELanguage::German,   "Steuerung anpassen" },
		{ ELanguage::French,   "Personnaliser les contrôles" },
		{ ELanguage::Spanish,  "Personalizar los controles" },
		{ ELanguage::Italian,  "Personalizzazione Input" }
	}
},
{
	"Options_Desc_CustomizeInput",
	{
		{ ELanguage::English,  "Customize controls for each player." },
		{ ELanguage::Japanese, "プレイヤーごとに操作をカスタマイズします。" },
		{ ELanguage::German,   "Anpassung der Steuerung für Individuelle Spieler." },
		{ ELanguage::French,   "Personnalisez les commandes pour chaque joueur." },
		{ ELanguage::Spanish,  "Personaliza los controles para cada jugador." },
		{ ELanguage::Italian,  "Personalizza i controlli per ciascun giocatore." }
	}
},
{
	"Options_Category_Keyboard",
	{
		{ ELanguage::English,  "KEYBOARD" },
		{ ELanguage::Japanese, "キーボード" },
		{ ELanguage::German,   "TASTATUR" },
		{ ELanguage::French,   "CLAVIER" },
		{ ELanguage::Spanish,  "TECLADO" },
		{ ELanguage::Italian,  "TASTIERA" }
	}
},
{
	"Options_Desc_Category_Keyboard",
	{
		{ ELanguage::English,  "Customize keyboard controls." },
		{ ELanguage::Japanese, "キーボード操作をカスタマイズします。" },
		{ ELanguage::German,   "Tastatursteuerung anpassen." },
		{ ELanguage::French,   "Personnaliser les contrôles du clavier." },
		{ ELanguage::Spanish,  "Personaliza los controles del teclado." },
		{ ELanguage::Italian,  "Personalizza i controlli per Tastiera." }
	}
},
{
	// Notes: {} is the player number (1P, 2P, 3P, 4P)
	"Options_Category_Gamepad",
	{
		{ ELanguage::English,  "{}P GAMEPAD" },
		{ ELanguage::Japanese, "{}P ゲームパッド" },
		{ ELanguage::German,   "{}SP GAMEPAD" },
		{ ELanguage::French,   "J{} MANETTE" },
		{ ELanguage::Spanish,  "J{} MANDO" },
		{ ELanguage::Italian,  "{}P GAMEPAD" }
	}
},
{
	"Options_Desc_Category_Gamepad",
	{
		{ ELanguage::English,  "Customize {}P gamepad buttons." },
		{ ELanguage::Japanese, "{}Pのゲームパッドのボタンをカスタマイズします。" },
		{ ELanguage::German,   "{}SP Controller steuerung anpassen." },
		{ ELanguage::French,   "Personnaliser les boutons de la manette de J{}." },
		{ ELanguage::Spanish,  "Personaliza los botones del mando del J{}." },
		{ ELanguage::Italian,  "Personalizza i tasti del controller {}P." }
	}
},
{
	"Options_Desc_BindKey",
	{
		{ ELanguage::English,  "Change key assignment." },
		{ ELanguage::Japanese, "キーの割り当てを変更します。" },
		{ ELanguage::German,   "Tastenbelegung ändern." },
		{ ELanguage::French,   "Modifie l'assignation des touches." },
		{ ELanguage::Spanish,  "Cambia la asignación de teclas." },
		{ ELanguage::Italian,  "Cambia il tasto assegnato." }
	}
},
{
	"Options_Desc_BindButton",
	{
		{ ELanguage::English,  "Change assigned button." },
		{ ELanguage::Japanese, "割り当てるボタンを変更します。" },
		{ ELanguage::German,   "Knopfbelegung ändern." },
		{ ELanguage::French,   "Modifie l'assignation des boutons." },
		{ ELanguage::Spanish,  "Cambia el botón asignado." },
		{ ELanguage::Italian,  "Cambia il pulsante assegnato." }
	}
},
{
	"Options_Desc_BindKeyListening",
	{
		{ ELanguage::English,  "Press key to assign. Press Escape or wait to cancel." },
		{ ELanguage::Japanese, "割り当てるキーを押してください。キャンセルするにはEscapeキーを押すか、そのままお待ちください。" },
		{ ELanguage::German,   "Taste drücken zum Zuweisen. Drücke Escape oder warte um zurückzukehren." },
		{ ELanguage::French,   "Appuyez sur une touche pour assigner. Appuyez sur Escape ou attendez pour annuler." },
		{ ELanguage::Spanish,  "Pulsa una tecla para asignar. Pulsa Escape o espera para cancelar." },
		{ ELanguage::Italian,  "Premi un tasto per assegnare. Premi Escape o aspetta per annullare." }
	}
},
{
	"Options_Desc_BindButtonListening",
	{
		{ ELanguage::English,  "Press button to assign. Press Escape or wait to cancel." },
		{ ELanguage::Japanese, "割り当てるボタンを押してください。キャンセルするにはEscapeキーを押すか、そのままお待ちください。" },
		{ ELanguage::German,   "Knopf drücken zum Zuweisen. Drücke Escape oder warte um zurückzukehren." },
		{ ELanguage::French,   "Appuyez sur un bouton pour assigner. Appuyez sur Escape ou attendez pour annuler." },
		{ ELanguage::Spanish,  "Pulsa un botón para asignar. Pulsa Escape o espera para cancelar." },
		{ ELanguage::Italian,  "Premi un pulsante per assegnare. Premi Escape o aspetta per annullare." }
	}
},
{
	"Input_Button_LeftTrigger",
	{
		{ ELanguage::English,  "LEFT TRIGGER" },
		{ ELanguage::Japanese, "左トリガ" },
		{ ELanguage::German,   "LINKER TRIGGER" },
		{ ELanguage::French,   "GACHETTE GAUCHE" },
		{ ELanguage::Spanish,  "GATILLO IZQUIERDO" },
		{ ELanguage::Italian,  "TRIGGER SINISTRO" },
	}
},
{
	"Input_Button_RightTrigger",
	{
		{ ELanguage::English,  "RIGHT TRIGGER" },
		{ ELanguage::Japanese, "右トリガ" },
		{ ELanguage::German,   "RECHTER TRIGGER" },
		{ ELanguage::French,   "GACHETTE DROITE" },
		{ ELanguage::Spanish,  "GATILLO DERECHO" },
		{ ELanguage::Italian,  "TRIGGER DESTRO" }
	}
},
{
	"Input_Button_DPadUp",
	{
		{ ELanguage::English,  "D-PAD UP" },
		{ ELanguage::Japanese, "十字キー 上" },
		{ ELanguage::German,   "D-PAD HOCH" },
		{ ELanguage::French,   "D-PAD HAUT" },
		{ ELanguage::Spanish,  "CRUCETA ARRIBA" },
		{ ELanguage::Italian,  "D-PAD SU" }
	}
},
{
	"Input_Button_DPadDown",
	{
		{ ELanguage::English,  "D-PAD DOWN" },
		{ ELanguage::Japanese, "十字キー 下" },
		{ ELanguage::German,   "D-PAD RUNTER" },
		{ ELanguage::French,   "D-PAD BAS" },
		{ ELanguage::Spanish,  "CRUCETA ABAJO" },
		{ ELanguage::Italian,  "D-PAD GIÙ" }
	}
},
{
	"Input_Button_DPadLeft",
	{
		{ ELanguage::English,  "D-PAD LEFT" },
		{ ELanguage::Japanese, "十字キー 左" },
		{ ELanguage::German,   "D-PAD LINKS" },
		{ ELanguage::French,   "D-PAD GAUCHE" },
		{ ELanguage::Spanish,  "CRUCETA IZQUIERDA" },
		{ ELanguage::Italian,  "D-PAD SINISTRO" }
	}
},
{
	"Input_Button_DPadRight",
	{
		{ ELanguage::English,  "D-PAD RIGHT" },
		{ ELanguage::Japanese, "十字キー 右" },
		{ ELanguage::German,   "D-PAD RECHTS" },
		{ ELanguage::French,   "D-PAD DROITE" },
		{ ELanguage::Spanish,  "CRUCETA DERECHA" },
		{ ELanguage::Italian,  "D-PAD DESTRO" }
	}
},
{
	"Input_Button_White",
	{
		{ ELanguage::English,  "WHITE" },
		{ ELanguage::Japanese, "白ボタン" },
		{ ELanguage::German,   "WEIßER KNOPF" },
		{ ELanguage::French,   "BLANC" },
		{ ELanguage::Spanish,  "BLANCO" },
		{ ELanguage::Italian,  "BIANCO" }
	}
},
{
	"Input_Button_Black",
	{
		{ ELanguage::English,  "BLACK" },
		{ ELanguage::Japanese, "黒ボタン" },
		{ ELanguage::German,   "SCHWARZER KNOPF" },
		{ ELanguage::French,   "NOIR" },
		{ ELanguage::Spanish,  "NEGRO" },
		{ ELanguage::Italian,  "NERO" }
	}
},
{
	"Input_Button_LeftStickClick",
	{
		{ ELanguage::English,  "LEFT STICK CLICK" },
		{ ELanguage::Japanese, "左スティッククリック" },
		{ ELanguage::German,   "L-STICK KLICK" },
		{ ELanguage::French,   "CLIC STICK GAUCHE" },
		{ ELanguage::Spanish,  "CLIC DEL STICK IZQUIERDO" },
		{ ELanguage::Italian,  "CLICK STICK SINISTRO" }
	}
},
{
	"Input_Button_RightStickClick",
	{
		{ ELanguage::English,  "RIGHT STICK CLICK" },
		{ ELanguage::Japanese, "右スティッククリック" },
		{ ELanguage::German,   "R-STICK KLICK" },
		{ ELanguage::French,   "CLIC STICK DROIT" },
		{ ELanguage::Spanish,  "CLIC DEL STICK DERECHO" },
		{ ELanguage::Italian,  "CLICK STICK DESTRO" }
	}
},
{
	"Input_Button_LeftStickUp",
	{
		{ ELanguage::English,  "LEFT STICK UP" },
		{ ELanguage::Japanese, "左スティック 上" },
		{ ELanguage::German,   "L-STICK HOCH" },
		{ ELanguage::French,   "STICK GAUCHE HAUT" },
		{ ELanguage::Spanish,  "STICK IZQUIERDO ARRIBA" },
		{ ELanguage::Italian,  "STICK SINISTRO SU" }
	}
},
{
	"Input_Button_LeftStickDown",
	{
		{ ELanguage::English,  "LEFT STICK DOWN" },
		{ ELanguage::Japanese, "左スティック 下" },
		{ ELanguage::German,   "L-STICK RUNTER" },
		{ ELanguage::French,   "STICK GAUCHE BAS" },
		{ ELanguage::Spanish,  "STICK IZQUIERDO ABAJO" },
		{ ELanguage::Italian,  "STICK SINISTRO GIÙ" }
	}
},
{
	"Input_Button_LeftStickLeft",
	{
		{ ELanguage::English,  "LEFT STICK LEFT" },
		{ ELanguage::Japanese, "左スティック 左" },
		{ ELanguage::German,   "L-STICK LINKS" },
		{ ELanguage::French,   "STICK GAUCHE GAUCHE" },
		{ ELanguage::Spanish,  "STICK IZQUIERDO IZQUIERDA" },
		{ ELanguage::Italian,  "STICK SINISTRO SINISTRA" }
	}
},
{
	"Input_Button_LeftStickRight",
	{
		{ ELanguage::English,  "LEFT STICK RIGHT" },
		{ ELanguage::Japanese, "左スティック 右" },
		{ ELanguage::German,   "L-STICK RECHTS" },
		{ ELanguage::French,   "STICK GAUCHE DROIT" },
		{ ELanguage::Spanish,  "STICK IZQUIERDO DERECHA" },
		{ ELanguage::Italian,  "STICK SINISTRO DESTRA" }
	}
},
{
	"Input_Button_RightStickUp",
	{
		{ ELanguage::English,  "RIGHT STICK UP" },
		{ ELanguage::Japanese, "右スティック 上" },
		{ ELanguage::German,   "R-STICK HOCH" },
		{ ELanguage::French,   "STICK DROIT HAUT" },
		{ ELanguage::Spanish,  "STICK DERECHO ARRIBA" },
		{ ELanguage::Italian,  "STICK DESTRO SU" }
	}
},
{
	"Input_Button_RightStickDown",
	{
		{ ELanguage::English,  "RIGHT STICK DOWN" },
		{ ELanguage::Japanese, "右スティック 下" },
		{ ELanguage::German,   "R-STICK RUNTER" },
		{ ELanguage::French,   "STICK DROIT BAS" },
		{ ELanguage::Spanish,  "STICK DERECHO ABAJO" },
		{ ELanguage::Italian,  "STICK DESTRO GIÙ" }
	}
},
{
	"Input_Button_RightStickLeft",
	{
		{ ELanguage::English,  "RIGHT STICK LEFT" },
		{ ELanguage::Japanese, "右スティック 左" },
		{ ELanguage::German,   "R-STICK LINKS" },
		{ ELanguage::French,   "STICK DROIT GAUCHE" },
		{ ELanguage::Spanish,  "STICK DERECHO IZQUIERDA" },
		{ ELanguage::Italian,  "STICK DESTRO SINISTRA" }
	}
},
{
	"Input_Button_RightStickRight",
	{
		{ ELanguage::English,  "RIGHT STICK RIGHT" },
		{ ELanguage::Japanese, "右スティック 右" },
		{ ELanguage::German,   "R-STICK RECHTS" },
		{ ELanguage::French,   "STICK DROIT DROIT" },
		{ ELanguage::Spanish,  "STICK DERECHO DERECHA" },
		{ ELanguage::Italian,  "STICK DESTRO DESTRA" }
	}
},
{
	"Input_Button_PS_Cross",
	{
		{ ELanguage::English,  "CROSS" },
		{ ELanguage::Japanese, "×ボタン" },
		{ ELanguage::German,   "KREUZ" },
		{ ELanguage::French,   "CROIX" },
		{ ELanguage::Spanish,  "CRUZ" },
		{ ELanguage::Italian,  "CROCE" }
	}
},
{
	"Input_Button_PS_Circle",
	{
		{ ELanguage::English,  "CIRCLE" },
		{ ELanguage::Japanese, "〇ボタン" },
		{ ELanguage::German,   "KREIS" },
		{ ELanguage::French,   "CERCLE" },
		{ ELanguage::Spanish,  "CÍRCULO" },
		{ ELanguage::Italian,  "CERCHIO" }
	}
},
{
	"Input_Button_PS_Square",
	{
		{ ELanguage::English,  "SQUARE" },
		{ ELanguage::Japanese, "□ボタン" },
		{ ELanguage::German,   "QUADRAT" },
		{ ELanguage::French,   "CARRÉ" },
		{ ELanguage::Spanish,  "CUADRADO" },
		{ ELanguage::Italian,  "QUADRATO" }
	}
},
{
	"Input_Button_PS_Triangle",
	{
		{ ELanguage::English,  "TRIANGLE" },
		{ ELanguage::Japanese, "△ボタン" },
		{ ELanguage::German,   "DREIECK" },
		{ ELanguage::French,   "TRIANGLE" },
		{ ELanguage::Spanish,  "TRIÁNGULO" },
		{ ELanguage::Italian,  "TRIANGOLO" }
	}
},
{
	"Input_Key_Return",
	{
		{ ELanguage::English,  "RETURN" },
		{ ELanguage::Japanese, "エンター" },
		{ ELanguage::German,   "ENTER" },
		{ ELanguage::French,   "ENTRÉE" },
		{ ELanguage::Spanish,  "ATRÁS" },
		{ ELanguage::Italian,  "RETURN" }
	}
},
{
	"Input_Key_Escape",
	{
		{ ELanguage::English,  "ESCAPE" },
		{ ELanguage::Japanese, "エスケープ" },
		{ ELanguage::German,   "ESCAPE" },
		{ ELanguage::French,   "ECHAP" },
		{ ELanguage::Spanish,  "ESCAPE" },
		{ ELanguage::Italian,  "ESCAPE" }
	}
},
{
	"Input_Key_Backspace",
	{
		{ ELanguage::English,  "BACKSPACE" },
		{ ELanguage::Japanese, "バックスペース" },
		{ ELanguage::German,   "RÜCKTASTE" },
		{ ELanguage::French,   "RETOUR" },
		{ ELanguage::Spanish,  "RETROCESO" },
		{ ELanguage::Italian,  "BACKSPACE" }
	}
},
{
	"Input_Key_Space",
	{
		{ ELanguage::English,  "SPACE" },
		{ ELanguage::Japanese, "スペース" },
		{ ELanguage::German,   "LEERTASTE" },
		{ ELanguage::French,   "ESPACE" },
		{ ELanguage::Spanish,  "ESPACIO" },
		{ ELanguage::Italian,  "SPAZIO" }
	}
},
{
	"Input_Key_Up",
	{
		{ ELanguage::English,  "UP" },
		{ ELanguage::Japanese, "上" },
		{ ELanguage::German,   "HOCH" },
		{ ELanguage::French,   "HAUT" },
		{ ELanguage::Spanish,  "ARRIBA" },
		{ ELanguage::Italian,  "SU" }
	}
},
{
	"Input_Key_Down",
	{
		{ ELanguage::English,  "DOWN" },
		{ ELanguage::Japanese, "下" },
		{ ELanguage::German,   "RUNTER" },
		{ ELanguage::French,   "BAS" },
		{ ELanguage::Spanish,  "ABAJO" },
		{ ELanguage::Italian,  "Giù" }
	}
},
{
	"Input_Key_Left",
	{
		{ ELanguage::English,  "LEFT" },
		{ ELanguage::Japanese, "左" },
		{ ELanguage::German,   "LINKS" },
		{ ELanguage::French,   "GAUCHE" },
		{ ELanguage::Spanish,  "IZQUIERDA" },
		{ ELanguage::Italian,  "SINISTRA" }
	}
},
{
	"Input_Key_Right",
	{
		{ ELanguage::English,  "RIGHT" },
		{ ELanguage::Japanese, "右" },
		{ ELanguage::German,   "RECHTS" },
		{ ELanguage::French,   "DROITE" },
		{ ELanguage::Spanish,  "DERECHA" },
		{ ELanguage::Italian,  "DESTRA" }
	}
},
{
	"Input_Key_LeftCtrl",
	{
		{ ELanguage::English,  "LEFT CTRL" },
		{ ELanguage::Japanese, "左コントロール" },
		{ ELanguage::German,   "STRG LINKS" },
		{ ELanguage::French,   "CTRL GAUCHE" },
		{ ELanguage::Spanish,  "CONTROL IZQUIERDO" },
		{ ELanguage::Italian,  "CNTRL SINISTRO" }
	}
},
{
	"Input_Key_RightCtrl",
	{
		{ ELanguage::English,  "RIGHT CTRL" },
		{ ELanguage::Japanese, "右コントロール" },
		{ ELanguage::German,   "STRG RECHTS" },
		{ ELanguage::French,   "CTRL DROIT" },
		{ ELanguage::Spanish,  "CONTROL DERECHO" },
		{ ELanguage::Italian,  "CNTRL DESTRO" }
	}
},
{
	"Input_Key_LeftShift",
	{
		{ ELanguage::English,  "LEFT SHIFT" },
		{ ELanguage::Japanese, "左シフト" },
		{ ELanguage::German,   "UMSCHALT LINKS" },
		{ ELanguage::French,   "MAJ." },
		{ ELanguage::Spanish,  "SHIFT IZQUIERDO" },
		{ ELanguage::Italian,  "SHIFT SINISTRO" }
	}
},
{
	"Input_Key_RightShift",
	{
		{ ELanguage::English,  "RIGHT SHIFT" },
		{ ELanguage::Japanese, "右シフト" },
		{ ELanguage::German,   "UMSCHALT RECHTS" },
		{ ELanguage::French,   "MAJ DROITE" },
		{ ELanguage::Spanish,  "SHIFT DERECHO" },
		{ ELanguage::Italian,  "SHIFT DESTRO" }
	}
},
{
	"Input_Key_LeftAlt",
	{
		{ ELanguage::English,  "LEFT ALT" },
		{ ELanguage::Japanese, "左オルト" },
		{ ELanguage::German,   "ALT LINKS" },
		{ ELanguage::French,   "ALT" },
		{ ELanguage::Spanish,  "ALT IZQUIERDO" },
		{ ELanguage::Italian,  "ALT SINISTRO" }
	}
},
{
	"Input_Key_RightAlt",
	{
		{ ELanguage::English,  "RIGHT ALT" },
		{ ELanguage::Japanese, "右オルト" },
		{ ELanguage::German,   "ALT RECHTS" },
		{ ELanguage::French,   "ALT GR" },
		{ ELanguage::Spanish,  "ALT GR" },
		{ ELanguage::Italian,  "ALT DESTRO" }
	}
},
{
	"Options_Name_SoundTest",
	{
		{ ELanguage::English,  "Sound Test" },
		{ ELanguage::Japanese, "サウンドテスト" },
		{ ELanguage::German,   "Sound Test" },
		{ ELanguage::French,   "Test Audio" },
		{ ELanguage::Spanish,  "Prueba de sonido" },
		{ ELanguage::Italian,  "Test Audio" }
	}
},
{
	"Options_Desc_SoundTest",
	{
		{ ELanguage::English,  "Listen to the in-game music." },
		{ ELanguage::Japanese, "ゲーム中に登場したサウンドを確認します。" },
		{ ELanguage::German,   "Höre die Musik des Spiels an." },
		{ ELanguage::French,   "Écoutez la musique du jeu." },
		{ ELanguage::Spanish,  "Escucha la música del juego." },
		{ ELanguage::Italian,  "Ascolta la musica del gioco." }
	}
},
{
	"Options_Desc_SoundTest_Track",
	{
		{ ELanguage::English,  "Select a track to play it." },
		{ ELanguage::Japanese, "再生するサウンドを選択してください。" },
		{ ELanguage::German,   "Höre dir die Musik aus dem Spiel an." },
		{ ELanguage::French,   "Sélectionnez une musique pour la jouer." },
		{ ELanguage::Spanish,  "Selecciona una canción para escuchar." },
		{ ELanguage::Italian,  "Seleziona una traccia per ascoltare." },
	}
},
{
	// Notes: shown next to the sound test track that is currently playing.
	"Options_Value_Playing",
	{
		{ ELanguage::English,  "Playing" },
		{ ELanguage::Japanese, "再生中" },
		{ ELanguage::German,   "Läuft" },
		{ ELanguage::French,   "Lecture" },
		{ ELanguage::Spanish,  "Reproduciendo" },
		{ ELanguage::Italian,  "In riproduzione" }
	}
},
{
	// Notes: description for options that cannot be accessed during gameplay (e.g. Language).
	"Options_Desc_NotAvailable",
	{
		{ ELanguage::English,  "This option is not available during gameplay." },
		{ ELanguage::Japanese, "このオプションはゲームプレイ中は使用できません。" },
		{ ELanguage::German,   "Diese Einstellung kann während des Spiels nicht verändert werden." },
		{ ELanguage::French,   "Cette option n'est pas disponible en cours de jeu." },
		{ ELanguage::Spanish,  "Esta opción no está disponible durante la partida." },
		{ ELanguage::Italian,  "Questa opzione non è disponibile durante il gioco." }
	}
},
{
	// Notes: currently the description for Window Size when in fullscreen.
	"Options_Desc_NotAvailableFullscreen",
	{
		{ ELanguage::English,  "This option is not available in fullscreen mode." },
		{ ELanguage::Japanese, "このオプションはフルスクリーンモードでは変更できません。" },
		{ ELanguage::German,   "Diese Option ist im Vollbildmodus nicht verfügbar." },
		{ ELanguage::French,   "Cette option n'est pas disponible en mode plein écran." },
		{ ELanguage::Spanish,  "Esta opción no está disponible en pantalla completa." },
		{ ELanguage::Italian,  "Questa opzione non è disponibile in modalità schermo pieno." }
	}
},
{
	// Notes: currently the description for Monitor when in fullscreen.
	"Options_Desc_NotAvailableWindowed",
	{
		{ ELanguage::English,  "This option is not available in windowed mode." },
		{ ELanguage::Japanese, "このオプションはウィンドウモードでは変更できません。" },
		{ ELanguage::German,   "Diese Option ist im Fenstermodus nicht verfügbar." },
		{ ELanguage::French,   "Cette option n'est pas disponible en mode fenêtré." },
		{ ELanguage::Spanish,  "Esta opción no está disponible en ventana." },
		{ ELanguage::Italian,  "Questa opzione non è disponibile in modalità finestra." }
	}
},
{
	// Notes: currently the description for Monitor when the user only has one display connected.
	"Options_Desc_NotAvailableHardware",
	{
		{ ELanguage::English,  "This option is not available with your current hardware configuration." },
		{ ELanguage::Japanese, "このオプションは現在のハードウェア構成で変更できません。" },
		{ ELanguage::German,   "Diese Option ist mit ihrer Hardwarekonfiguration nicht verfügbar." },
		{ ELanguage::French,   "Cette option n'est pas disponible avec votre configuration matérielle actuelle." },
		{ ELanguage::Spanish,  "Esta opción no está disponible con tu configuración actual de hardware." },
		{ ELanguage::Italian,  "Questa opzione non è disponibile con l'hardware in tuo possesso." }
	}
},
{
	// Notes: message appears when changing the Language option and backing out of the options menu.
	"Options_Message_Restart",
	{
		{ ELanguage::English,  "The game will now restart." },
		{ ELanguage::Japanese, "ゲームが再起動されます。" },
		{ ELanguage::German,   "Das Spiel wird jetzt neu starten." },
		{ ELanguage::French,   "Le jeu va maintenant redémarrer." },
		{ ELanguage::Spanish,  "El juego se va a reiniciar." },
		{ ELanguage::Italian,  "Il gioco verrà riavviato." }
	}
},
{
	// Notes: text in pause menu during gameplay
	"Pause_Menu_Continue",
	{
		{ ELanguage::English,  "resume" },
		{ ELanguage::Japanese, "つづける" },
		{ ELanguage::German,   "weiter" },
		{ ELanguage::French,   "reprendre" },
		{ ELanguage::Spanish,  "continuar" },
		{ ELanguage::Italian,  "continua" }
	}
},
{
	// Notes: text in pause menu during gameplay
	"Pause_Menu_Restart",
	{
		{ ELanguage::English,  "restart" },
		{ ELanguage::Japanese, "はじめから" },
		{ ELanguage::German,   "neustart" },
		{ ELanguage::French,   "redémarrer" },
		{ ELanguage::Spanish,  "reiniciar" },
		{ ELanguage::Italian,  "ricomincia" }
	}
},
{
	// Notes: text in pause menu during gameplay
	"Pause_Menu_Options",
	{
		{ ELanguage::English,  "options" },
		{ ELanguage::Japanese, "オプション" },
		{ ELanguage::German,   "optionen" },
		{ ELanguage::French,   "options" },
		{ ELanguage::Spanish,  "opciones" },
		{ ELanguage::Italian,  "opzioni" }
	}
},
{
	// Notes: text in pause menu during gameplay
	"Pause_Menu_Quit",
	{
		{ ELanguage::English,  "quit" },
		{ ELanguage::Japanese, "やめる" },
		{ ELanguage::German,   "beenden" },
		{ ELanguage::French,   "quitter" },
		{ ELanguage::Spanish,  "abandonar" },
		{ ELanguage::Italian,  "abbandona" }
	}
},
{
	// Notes: used for the button guide at the pause menu.
	"Achievements_Name",
	{
		{ ELanguage::English,  "achievements" },
		{ ELanguage::Japanese, "じっせき" },
		{ ELanguage::German,   "erfolge" },
		{ ELanguage::French,   "succès" },
		{ ELanguage::Spanish,  "logros" },
		{ ELanguage::Italian,  "obiettivi" }
	}
},
{
	"Achievements_Unlock",
	{
		{ ELanguage::English,  "Achievement Unlocked!" },
		{ ELanguage::Japanese, "実績を解除しました！" },
		{ ELanguage::German,   "Erfolg Freigeschaltet!" },
		{ ELanguage::French,   "Succès déverrouillé !" },
		{ ELanguage::Spanish,  "¡Logro desbloqueado!" },
		{ ELanguage::Italian,  "Obiettivo sbloccato!" }
	}
},
{
	"Installer_Header_Browse",
	{
		{ ELanguage::English,  "Browse..." },
		{ ELanguage::Japanese, "ブラウズ..." },
		{ ELanguage::German,   "Durchsuchen..." },
		{ ELanguage::French,   "Parcourir..." },
		{ ELanguage::Spanish,  "Explorar..." },
		{ ELanguage::Italian,  "Esplora..." },
	}
},
{
	"Installer_Page_SelectLanguage",
	{
		{ ELanguage::English,  "Please select a language." },
		{ ELanguage::Japanese, "言語を選択してください。" },
		{ ELanguage::German,   "Bitte eine Sprache auswählen." },
		{ ELanguage::French,   "Choisissez une langue." },
		{ ELanguage::Spanish,  "Por favor, selecciona un idioma." },
		{ ELanguage::Italian,  "Seleziona una lingua." }
	}
},
{
	"Installer_Page_Introduction",
	{
		{ ELanguage::English,  "Welcome to Heroes Recompiled!\n\nYou'll need an Xbox copy of\nSonic Heroes in order to\nproceed with the installation." },
		{ ELanguage::Japanese, "Heroes Recompiledへようこそ！\n\nインストールを続行するには、\n『ソニックヒーローズ』の\nXbox版が必要です。" },
		{ ELanguage::German,   "Wilkommen zu Heroes Recompiled!\n\nDu wirst eine Xbox Kopie von\nSonic Heroes benötigen um\nmit der Installation fortzufahren."},
		{ ELanguage::French,   "Bienvenue dans Heroes Recompiled!\n\nVous aurez besoin d'une copie Xbox\nde Sonic Heroes pour\npouvoir continuer l'installation." },
		{ ELanguage::Spanish,  "¡Te damos la bienvenida a\nHeroes Recompiled!\n\nNecesitarás una copia de la versión de XBOX de Sonic Heroes\npara proceder con la instalación." },
		{ ELanguage::Italian,  "Benvenuti a Heroes Recompiled!\n\nTi servirà una copia XBOX di\nSonic Heroes per poter\nProcedere con l'installazione" }
	}
},
{
	"Installer_Page_SelectGame",
	{
		{ ELanguage::English,  "Add the source for the game." },
		{ ELanguage::Japanese, "ゲームの元データを追加してください。" },
		{ ELanguage::German,   "Füge den Pfad für das Spiel hinzu." },
		{ ELanguage::French,   "Ajouter les fichiers du jeu." },
		{ ELanguage::Spanish,  "Añade el archivo de origen del juego." },
		{ ELanguage::Italian,  "Aggiungi una fonte per il gioco." }
	}
},
{
	"Installer_Page_NoFileSelected",
	{
		{ ELanguage::English,  "Drag & Drop or Select an ISO or default.xbe" },
		{ ELanguage::Japanese, "ISOまたはdefault.xbeを\nドラッグ＆ドロップするか、選択してください。" },
		{ ELanguage::German,   "Drag & Drop oder wähle eine ISO oder default.xbe aus." },
		{ ELanguage::French,   "Faites glisser ou sélectionnez le fichier .ISO ou default.xbe" },
		{ ELanguage::Spanish,  "Arrasta o selecciona el archivo ISO\no default.xbe"  },
		{ ELanguage::Italian,  "Trascina e rilascia o seleziona una ISO o default.xbe" }
	}
},
{
		"Installer_Page_CheckSpace",
		{
			{ ELanguage::English,  "The content will be installed to the program's folder.\n\n" },
			{ ELanguage::Japanese, "コンテンツはプログラムのフォルダに\nインストールされます。\n" },
			{ ELanguage::German,   "Der Inhalt wird in dem Ordner des Programms installiert.\n" },
			{ ELanguage::French,   "Le contenu sera installé dans le même dossier que le programme.\n" },
			{ ELanguage::Spanish,  "El contenido será instalado en la carpeta del programa.\n\n" },
			{ ELanguage::Italian,  "Il contenuto verrà installato nella cartella di questo programma.\n\n" }
		}
},
{
		"Installer_Step_RequiredSpace",
		{
			{ ELanguage::English,  "Required space: %.2f GiB" },
			{ ELanguage::Japanese, "必要な容量: %.2f GiB" },
			{ ELanguage::German,   "Benötigter Speicherplatz:\n%.2f GiB" },
			{ ELanguage::French,   "Espace nécessaire :\n%.2f Gio" },
			{ ELanguage::Spanish,  "Espacio necesario: %.2f GB" },
			{ ELanguage::Italian,  "Spazio necessario: %.2f GiB" }
		}
},
{
		"Installer_Step_AvailableSpace",
		{
			{ ELanguage::English,  "Available space: %.2f GiB" },
			{ ELanguage::Japanese, "使用可能な容量: %.2f GiB" },
			{ ELanguage::German,   "Verfügbarer Speicherplatz:\n%.2f GiB" },
			{ ELanguage::French,   "Espace disponible :\n%.2f Gio" },
			{ ELanguage::Spanish,  "Espacio disponible: %.2f GB" },
			{ ELanguage::Italian,  "Spazio disponibile: %.2f GiB" }
		}
},
	{
		"Installer_Page_Installing",
		{
			{ ELanguage::English,  "Installing..." },
			{ ELanguage::Japanese, "インストール中..." },
			{ ELanguage::German,   "Installiert..." },
			{ ELanguage::French,   "Installation..." },
			{ ELanguage::Spanish,  "Instalando..." },
			{ ELanguage::Italian,  "Installazione..." }
		}
	},
	{
		"Installer_Credits_Message",
		{
			{ ELanguage::English,  "Credits: " },
			{ ELanguage::Japanese, "クレジット： " },
			{ ELanguage::German,   "Credits: " },
			{ ELanguage::French,   "Crédits: " },
			{ ELanguage::Spanish,  "Créditos: " },
			{ ELanguage::Italian,  "Crediti: " }
		}
	},
{
	"Installer_Page_Compiling",
	{
		{ ELanguage::English,  "Compiling shaders..." },
		{ ELanguage::Japanese, "シェーダーをコンパイル中..." },
		{ ELanguage::German,   "Shaders kompilieren..." },
		{ ELanguage::French,   "Compilation des shaders..." },
		{ ELanguage::Spanish,  "Compilando sombreadores..." },
		{ ELanguage::Italian,  "Compilazione shaders..." }
	}
},
{
	"Installer_Page_InstallSucceeded",
	{
		{ ELanguage::English,  "Installation complete!\nThis project is brought to you by:" },
		{ ELanguage::Japanese, "インストール完了！\nプロジェクト制作：" },
		{ ELanguage::German,   "Installation abgeschlossen!\nDieses Projekt wird präsentiert von:" },
		{ ELanguage::French,   "Installation terminée !\nCe projet vous est présenté\npar :" },
		{ ELanguage::Spanish,  "¡Instalación completada!\nEste proyecto ha sido posible gracias a:" },
		{ ELanguage::Italian,  "Installazione completata!\nQuesto progetto è stato creato da:" }
	}
},
{
	"Installer_Page_InstallFailed",
	{
		{ ELanguage::English,  "Installation failed.\n\nAn unknown error occurred." },
		{ ELanguage::Japanese, "インストールに失敗しました。不明なエラーが発生しました。" },
		{ ELanguage::German,   "Installation fehlgeschlagen.\n\nEin unbekannter Fehler ist aufgetreten." },
		{ ELanguage::French,   "L'installation a échoué.\n\nUne erreur inconnue est survenue." },
		{ ELanguage::Spanish,  "La instalación ha fallado.\n\nOcurrió un error desconocido." },
		{ ELanguage::Italian,  "Installazione fallita.\n\nSi è verificato un errore sconosciuto." }
	}
},
{
	"Installer_Step_Game",
	{
		{ ELanguage::English,  "Game" },
		{ ELanguage::Japanese, "ゲーム" },
		{ ELanguage::German,   "Spiel" },
		{ ELanguage::French,   "Jeu" },
		{ ELanguage::Spanish,  "Juego" },
		{ ELanguage::Italian,  "Gioco" }
	}
},
{
	"Installer_Button_Next",
	{
		{ ELanguage::English,  "Next" },
		{ ELanguage::Japanese, "次へ" },
		{ ELanguage::German,   "Weiter" },
		{ ELanguage::French,   "Suivant" },
		{ ELanguage::Spanish,  "Siguiente" },
		{ ELanguage::Italian,  "Continua" }
	}
},
{
	"Installer_Button_Retry",
	{
		{ ELanguage::English,  "Retry" },
		{ ELanguage::Japanese, "リトライ" },
		{ ELanguage::German,   "Erneut versuchen" },
		{ ELanguage::French,   "Réessayer" },
		{ ELanguage::Spanish,  "Reintentar" },
		{ ELanguage::Italian,  "Riprova" }
	}
},
{
	"Installer_Button_Back",
	{
		{ ELanguage::English,  "Back" },
		{ ELanguage::Japanese, "戻る" },
		{ ELanguage::German,   "Zurück" },
		{ ELanguage::French,   "Retour" },
		{ ELanguage::Spanish,  "Atrás" },
		{ ELanguage::Italian,  "Indietro" }
	}
},
{
	"Installer_Button_Quit",
	{
		{ ELanguage::English,  "Quit" },
		{ ELanguage::Japanese, "終了" },
		{ ELanguage::German,   "Verlassen" },
		{ ELanguage::French,   "Quitter" },
		{ ELanguage::Spanish,  "Salir" },
		{ ELanguage::Italian,  "Esci" }
	}
},
{
	"Installer_Button_PlayGame",
	{
		{ ELanguage::English,  "Play Game" },
		{ ELanguage::Japanese, "ゲームを開始" },
		{ ELanguage::German,   "Spiel starten" },
		{ ELanguage::French,   "Jouer" },
		{ ELanguage::Spanish,  "Jugar" },
		{ ELanguage::Italian,  "Gioca" }
	}
},
{
	// Notes: message appears for any install failure
	"Installer_Message_InvalidFileSelected",
	{
		{ ELanguage::English,  "The selected source is not a valid Sonic Heroes Xbox ISO\nor extracted game folder." },
		{ ELanguage::Japanese, "選択した元データは、\n『ソニックヒーローズ』のXbox版ISOとしても、\n展開済みのゲームフォルダーとしても認識できません。" },
		{ ELanguage::German,   "Die ausgewählte Quelle ist keine gültige Sonic Heroes Xbox ISO-Datei\noder kein extrahierter Spielordner." },
		{ ELanguage::French,   "La fichier sélectionnée n'est pas un .ISO valide\nou un dossier du jeu Sonic Heroes." },
		{ ELanguage::Spanish,  "El archivo de origen seleccionado no es un archivo ISO válido de Sonic Heroes\npara Xbox ni una carpeta de juego extraída." },
		{ ELanguage::Italian,  "La fonte selezionata non è una XBOX ISO valida di Sonic Heroes\no una cartella di gioco estratta." }
	}
},
{
	// Notes: message appears when pressing B at the title screen.
	"Title_Message_Quit",
	{
		{ ELanguage::English,  "Are you sure you want to quit the game?" },
		{ ELanguage::Japanese, "終了してもよろしいですか？" },
		{ ELanguage::German,   "Spiel verlassen?" },
		{ ELanguage::French,   "Êtes-vous sûr de vouloir quitter le jeu ?" },
		{ ELanguage::Spanish,  "¿Seguro que quieres\nsalir?" },
		{ ELanguage::Italian,  "Sicuro di voler uscire ?" }
	}
},
{
	// Notes: message appears when creating a save file at the title screen.
	// Note, the game exclusively use "Game Data" when referring to save file.
	"Title_Message_CreateSave",
	{
		{ ELanguage::English,  "There is no Game Data on your PC.\nSHADOW THE HEDGEHOG™\nsupports an autosave feature.\nDo you want to create Game Data?" },
		{ ELanguage::Japanese, "シャドウ・ザ・ヘッジホッグの\nゲームデータが存在しません。\nシャドウ・ザ・ヘッジホッグは\nオートセーブを使用しています。\nゲームデータの作成を行います。\nよろしいですか？" },
		{ ELanguage::German,   "Auf Ihrem PC befinden sich keine Spieldaten.\nSHADOW THE HEDGEHOG™\nunterstützt eine automatische\nSpeicherfunktion. Möchten Sie Spieldaten erstellen?" },
		{ ELanguage::French,   "Pas de données de jeu sur ce PC.\nSHADOW THE HEDGEHOG™\ndispose d'un système de sauvegarde automatique.\nCréer des données de jeu sur\nce PC?" },
		{ ELanguage::Spanish,  "No se han encontrado datos de juego en tu PC.\nSHADOW THE HEDGEHOG™ utiliza una función de autoguardado.\n¿Quieres crear datos de juego?" },
		{ ELanguage::Italian,  "Non è stato trovato un salvataggio.\nSHADOW THE HEDGEHOG™\nsupporta una funzione di autosalvataggio.\nVuoi creare un nuovo salvataggio?" }
	}
},
{
	// Notes: message appears when creating a save file at the title screen.
	// Note, the game exclusively use "Game Data" when referring to save file.
	"Title_Message_CreateSaveFailed",
	{
		{ ELanguage::English,  "Start the game without creating\nGame Data.\nProgress of the game will not be saved." },
		{ ELanguage::Japanese, "ゲームデータを作成せずに開始します。\nこの場合、セーブはできません。" },
		{ ELanguage::German,   "Spiel starten, ohne Spieldaten zu\nerstellen. Der Spielstand wird\nnicht gespeichert." },
		{ ELanguage::French,   "Démarrer le jeu sans créer de données du jeu.\nLa progression du jeu ne sera\npas sauvegardée." },
		{ ELanguage::Spanish,  "Comenzarás el juego sin crear datos de juego.\nEl progreso durante la partida no será guardado." },
		{ ELanguage::Italian,  "Incomincia senza creare un\nSalvataggio.\nI progressi di gioco non saranno salvati." }
	}
},
// Notes: on load if the save file is corrupted
{
	"Title_Message_SaveDamaged",
	{
		{ ELanguage::English,  "The Game Data is damaged\nand cannot be used.\nPress A button to continue." },
		{ ELanguage::Japanese, "ゲームデータを読み込めません。\nゲームデータが破損しています。\nAボタンを押してください。" },
		{ ELanguage::German,   "Spieldaten sind beschädigt und\nkönnen nicht verwendet werden.\nDrücken Sie bitte die A-Taste\num fortzufahren.." },
		{ ELanguage::French,   "Les données de jeu sont endommagées.\nAppuyer sur A pour continuer." },
		{ ELanguage::Spanish,  "Los datos de juego están dañados y no se pueden utilizar.\nPulsa el botón A para continuar." },
		{ ELanguage::Italian,  "Il Salvataggio è danneggiato\ne non può essere usato.\nPremi A per continuare." },
	}
},
// Notes: same as above but without "Press A"
{
	"Title_Message_SaveDamagedNoA",
	{
		{ ELanguage::English,  "The Game Data is damaged\nand cannot be used." },
		{ ELanguage::Japanese, "ゲームデータを読み込めません。\nゲームデータが破損しています。" },
		{ ELanguage::German,   "Spieldaten sind beschädigt und\nkönnen nicht verwendet werden." },
		{ ELanguage::French,   "Les données de jeu sont endommagées." },
		{ ELanguage::Spanish,  "Los datos de juego están dañados y no se pueden utilizar." },
		{ ELanguage::Italian,  "Il salvataggio è danneggiato \ne e non può essere usato." },
	}
},
// Notes: If the game fails to save, rw lock or otherwise
{
	"Title_Message_SaveFailed",
	{
		{ ELanguage::English,  "Save failed." },
		{ ELanguage::Japanese, "セーブに失敗しました。" },
		{ ELanguage::German,   "Speichern gescheitert." },
		{ ELanguage::French,   "Echec de la sauvegarde." },
		{ ELanguage::Spanish,  "Error al guardar." },
		{ ELanguage::Italian,  "Salvataggio fallito." },
	}
},
{
	"Title_Message_OverwriteSave",
	{
		{ ELanguage::English,  "Do you wish to overwrite the Game Data?\nCaution! This will overwrite any\nprevious Game Data present." },
		{ ELanguage::Japanese, "ゲームデータに上書きします。\n上書きした場合、今までのゲームデータの\n内容は消えてしまいます。よろしいですか？\n以降はこのゲームデータに\nオートセーブを行います。" },
		{ ELanguage::German,   "Vorige Spieldaten sind auf dem Gerät\nschon vorhanden.\nspeichern wird sie überschrieben.\nSind Sie sicher das Sie Speichern\nmöchten?" },
		{ ELanguage::French,   "Voulez-vous écraser les données de jeu ?\nAttention, cela écrasera toutes les données de jeu précédentes." },
		{ ELanguage::Spanish,  "¿Quieres sobrescribir los datos de juego?\n¡Cuidado! Esto sobrescribirá los datos de juego existentes." },
		{ ELanguage::Italian,  "Desideri sovrascrivere il salvataggio?\nAttenzione! Questo sovrascrivera il\nsalvataggio di gioco presente" },
	}
},
{
	"Legacy_Options_Message_OverwriteSave",
	{
		{ ELanguage::English,  "Do you want to load Game Data?\nCaution! This will overwrite any\nprevious Game Data present." },
		{ ELanguage::Japanese, "ゲームデータをロードします。これまでの\nプレイ内容は上書きされるため失われて\nしまいます。よろしいですか？" },
		{ ELanguage::German,   "Spieldaten Laden?\nAchtung! Bereits vorhandene Spieldaten\nwerden überschrieben." },
		{ ELanguage::French,   "Voulez-vous charger les données de jeu ?\nAttention, cela écrasera toutes les données de jeu précédentes." },
		{ ELanguage::Spanish,  "¿Quieres cargar los datos de juego?\n¡Cuidado! Esto sobrescribirá los datos de juego existentes." },
		{ ELanguage::Italian,  "Vuoi caricare il salvataggio?\nAttenzione! Questo sovrascriverà\nil progresso di gioco presente." },
	}
},
{
	"Legacy_Options_Message_LoadSaveOK",
	{
		{ ELanguage::English,  "Load successful." },
		{ ELanguage::Japanese, "ゲームデータのロードが完了しました。" },
		{ ELanguage::German,   "Laden erfolgreicht." },
		{ ELanguage::French,   "Chargement réussi." },
		{ ELanguage::Spanish,  "Se han cargado correctamente." },
		{ ELanguage::Italian,  "Caricamento completato." },
	}
},
{
	// Notes: message appears when creating a save file at the title screen.
	// Note, the game exclusively use "Game Data" when referring to save file.
	"Title_Message_LoadSave",
	{
		{ ELanguage::English,  "Do you want to load Game Data?" },
		{ ELanguage::Japanese, "ゲームデータをロードします。\nよろしいですか？" },
		{ ELanguage::German,   "Spieldaten Laden?" },
		{ ELanguage::French,   "Souhaitez-vous charger les\n données de jeu?" },
		{ ELanguage::Spanish,  "¿Quieres cargar los datos de juego?" },
		{ ELanguage::Italian,  "Vuoi caricare i dati di gioco?" },
	}
},
{
	// Notes: message appears when creating a save file at the title screen.
	// Note, the game exclusively use "Game Data" when referring to save file.
	"Title_Message_ContinueWithoutSave",
	{
		// there is a space added before the question mark to match the original game text, even in english.
		{ ELanguage::English,  "Continue without saving ?" },
		{ ELanguage::Japanese, "ゲームデータをセーブせずに、\nこのままゲームを続けますか？\n（ゲームの途中経過などは保存されません。）" },
		{ ELanguage::German,   "Ohne Speichern fortfahren ?" },
		{ ELanguage::French,   "Continuez sans sauvegarder ?" },
		{ ELanguage::Spanish,  "¿ Quieres continuar sin guardar ?" },
		{ ELanguage::Italian,  "Continuare senza salvare ?" },
	}
},
{
	// Notes: message appears when choosing restart in a stage.
	"Pause_Message_Restart",
	{
		{ ELanguage::English,  "Are you sure you want to restart the\nstage? Your progress in the current\nstage will be lost if you restart." },
		{ ELanguage::Japanese, "このステージのスタート地点から、\nゲームをやり直します。\n現在プレイしているステージの\n内容は失われます。\nよろしいですか？" },
		{ ELanguage::German,   "Sind Sie sicher das Sie die Ebene  \nneu beginnen wollen? Der Spielfortschritt\ndieser Ebene wird dabei gelöscht." },
		{ ELanguage::French,   "Recommencer l'étape ?\nVotre progression sur cette étape\nsera perdue si vous recommencez." },
		{ ELanguage::Spanish,  "¿Quieres volver a empezar esta etapa?\nEl progreso en esta etapa se\nperderá si vuelves a empezar." },
		{ ELanguage::Italian,  "Sicuro di voler ricominciare il quadro?\nI progressi di questo quadro andranno\npersi se ricominci." },
	}
},
{
	// Notes: message appears when choosing quit in a stage.
	"Pause_Message_Quit",
	{
		{ ELanguage::English,  "Are you sure you want to quit the game?\nYour progress in the current stage\nwill be lost if you quit." },
		{ ELanguage::Japanese, "ゲームを途中で終了します。\n現在プレイしているステージの\n進行状況はセーブされません。\nよろしいですか？" },
		{ ELanguage::German,   "Sind Sie sicher das Sie das Spiel\nabbrechen wollen? \nSie verlieren dabei Ihren Fortschritt\nin dieser Ebene." },
		{ ELanguage::French,   "Quitter la partie ?\nVotre progression sur cette étape\nsera perdue si vous quittez." },
		{ ELanguage::Spanish,  "¿Quieres abandonar la partida?\nEl progreso de esta etapa se\nperderá si abandonas." },
		{ ELanguage::Italian,  "Sicuro di voler uscire dal gioco?\nI progressi di questo quadro andranno\npersi se esci dal gioco." },
	}
},
{
	"Unlock_Message_LastStory",
	{
		{ ELanguage::English,  "LAST STORY unlocked!" },
		{ ELanguage::Japanese, "LAST STORYが追加されました！" },
		{ ELanguage::German,   "LETZTE STORY freigeschaltet!" },
		{ ELanguage::French,   "DERNIERE HISTOIRE déverrouillée !" },
		{ ELanguage::Spanish,  "¡HISTORIA FINAL desbloqueada!" },
		{ ELanguage::Italian,  "STORIA FINALE sbloccata!" },
	}
},
{
	// Notes: the '.' in "EXPERT MODE." is an English only original typo
	"Unlock_Message_ExpertMode",
	{
		{ ELanguage::English,  "EXPERT MODE unlocked!" },
		{ ELanguage::Japanese, "ハードモードが遊べるようになりました！" },
		{ ELanguage::German,   "EXPERTEN-MODUS freigeschaltet!" },
		{ ELanguage::French,   "MODE EXPERT déverrouillé !" },
		{ ELanguage::Spanish,  "¡MODO EXPERTO desbloqueado!" },
		{ ELanguage::Italian,  "MODALITÀ ESPERTO sbloccata!" },
	}
},
{
	"Unlock_Message_SamuraiBladeLv1",
	{
		{ ELanguage::English,  "You unlocked \"Samurai Blade\"." },
		{ ELanguage::Japanese, "武器「サムライブレード」が\n使えるようになりました！" },
		{ ELanguage::German,   "Du hast \"Samurai Blade\" freigeschaltet." },
		{ ELanguage::French,   "Vous avez déverrouillé \"Samurai Blade\"." },
		{ ELanguage::Spanish,  "Has desbloqueado \"Samurai Blade\"." },
		{ ELanguage::Italian,  "Hai sbloccato \"Samurai Blade\"." },
	}
},
{
	"Unlock_Message_SamuraiBladeLv2",
	{
		{ ELanguage::English,  "\"Samurai Blade\" is now Level 2." },
		{ ELanguage::Japanese, "武器「サムライブレード」が\nレベル２になりました！" },
		{ ELanguage::German,   "\"Samurai Blade\" hat Level 2 erreicht." },
		{ ELanguage::French,   "Vous avez atteint le niveau 2 de \"Samurai Blade\"." },
		{ ELanguage::Spanish,  "\"Samurai Blade\" Nivel 2." },
		{ ELanguage::Italian,  "\"Samurai Blade\" ha raggiunto il livello 2." },
	}
},
{
	"Unlock_Message_SatelliteLaserLv1",
	{
		{ ELanguage::English,  "You unlocked \"Satellite Laser\"." },
		{ ELanguage::Japanese, "武器「軌道衛星レーザー」が\n使えるようになりました！" },
		{ ELanguage::German,   "Du hast \"Satellite Laser\" freigeschaltet." },
		{ ELanguage::French,   "Vous avez déverrouillé \"Satellite Laser\"." },
		{ ELanguage::Spanish,  "Has desbloqueado \"Satellite Laser\"." },
		{ ELanguage::Italian,  "Hai sbloccato \"Satellite Laser\"." },
	}
},
{
	"Unlock_Message_SatelliteLaserLv2",
	{
		{ ELanguage::English,  "\"Satellite Laser\" is now Level 2." },
		{ ELanguage::Japanese, "武器「軌道衛星レーザー」が\nレベル２になりました！" },
		{ ELanguage::German,   "\"Satellite Laser\" hat Level 2 erreicht." },
		{ ELanguage::French,   "Vous avez atteint le niveau 2 de \"Satellite Laser\"." },
		{ ELanguage::Spanish,  "\"Satellite Laser\" Nivel 2." },
		{ ELanguage::Italian,  "\"Satellite Laser\" ha raggiunto il livello 2." },
	}
},
{
	"Unlock_Message_VacuumEggLv1",
	{
		{ ELanguage::English,  "You unlocked \"Vacuum Egg\"." },
		{ ELanguage::Japanese, "武器「バキュームエッグ」が\n使えるようになりました！" },
		{ ELanguage::German,   "Du hast \"Vacuum Egg\" freigeschaltet." },
		{ ELanguage::French,   "Vous avez déverrouillé \"Vacuum Egg\"." },
		{ ELanguage::Spanish,  "Has desbloqueado \"Vacuum Egg\"." },
		{ ELanguage::Italian,  "Hai sbloccato \"Vacuum Egg\"." },
	}
},
{
	"Unlock_Message_VacuumEggLv2",
	{
		{ ELanguage::English,  "\"Vacuum Egg\" is now Level 2." },
		{ ELanguage::Japanese, "武器「バキュームエッグ」が\nレベル２になりました！" },
		{ ELanguage::German,   "\"Vacuum Egg\" hat Level 2 erreicht." },
		{ ELanguage::French,   "Vous avez atteint le niveau 2 de \"Vacuum Egg\"." },
		{ ELanguage::Spanish,  "\"Vacuum Egg\" Nivel 2." },
		{ ELanguage::Italian,  "\"Vacuum Egg\" ha raggiunto il livello 2." },
	}
},
{
	"Unlock_Message_OmochaoGunLv1",
	{
		{ ELanguage::English,  "You unlocked \"Omochao Gun\"." },
		{ ELanguage::Japanese, "武器「オモチャオガン」が\n使えるようになりました！" },
		{ ELanguage::German,   "Du hast \"Omochao Gun\" freigeschaltet." },
		{ ELanguage::French,   "Vous avez déverrouillé \"Omochao Gun\"." },
		{ ELanguage::Spanish,  "Has desbloqueado \"Omochao Gun\"." },
		{ ELanguage::Italian,  "Hai sbloccato \"Omochao Gun\"." },
	}
},
{
	"Unlock_Message_OmochaoGunLv2",
	{
		{ ELanguage::English,  "\"Omochao Gun\" is now Level 2." },
		{ ELanguage::Japanese, "武器「オモチャオガン」が\nレベル２になりました！" },
		{ ELanguage::German,   "\"Omochao Gun\" hat Level 2 erreicht." },
		{ ELanguage::French,   "Vous avez atteint le niveau 2 de \"Omochao Gun\"." },
		{ ELanguage::Spanish,  "\"Omochao Gun\" Nivel 2." },
		{ ELanguage::Italian,  "\"Omochao Gun\" ha raggiunto il livello 2." },
	}
},
{
	"Unlock_Message_HealCannonLv1",
	{
		{ ELanguage::English,  "You unlocked \"Heal Cannon\"." },
		{ ELanguage::Japanese, "武器「ヒールキャノン」が\n使えるようになりました！" },
		{ ELanguage::German,   "Du hast \"Heal Cannon\" freigeschaltet." },
		{ ELanguage::French,   "Vous avez déverrouillé \"Heal Cannon\"." },
		{ ELanguage::Spanish,  "Has desbloqueado \"Heal Cannon\"." },
		{ ELanguage::Italian,  "Hai sbloccato \"Heal Cannon\"." },
	}
},
{
	"Unlock_Message_HealCannonLv2",
	{
		{ ELanguage::English,  "\"Heal Cannon\" is now Level 2." },
		{ ELanguage::Japanese, "武器「ヒールキャノン」が\nレベル２になりました！" },
		{ ELanguage::German,   "\"Heal Cannon\" hat Level 2 erreicht." },
		{ ELanguage::French,   "Vous avez atteint le niveau 2 de \"Heal Cannon\"." },
		{ ELanguage::Spanish,  "\"Heal Cannon\" Nivel 2." },
		{ ELanguage::Italian,  "\"Heal Cannon\" ha raggiunto il livello 2." },
	}
},
{
	"Unlock_Message_ShadowRifle",
	{
		{ ELanguage::English,  "You unlocked \"Shadow Rifle\"." },
		{ ELanguage::Japanese, "武器「シャドウライフル」が\n使えるようになりました！" },
		{ ELanguage::German,   "Du hast \"Shadow Rifle\" freigeschaltet." },
		{ ELanguage::French,   "Vous avez déverrouillé \"Shadow Rifle\"." },
		{ ELanguage::Spanish,  "Has desbloqueado \"Shadow Rifle\"." },
		{ ELanguage::Italian,  "Hai sbloccato \"Shadow Rifle\"." },
	}
},
{
	"Title_Message_UpdateAvailable",
	{
		{ ELanguage::English,  "An update is available!\n\nWould you like to visit the\nreleases page to download it?" },
		{ ELanguage::Japanese, "アップデートが利用可能です！\n\nリリースページにアクセスしてダウンロードしますか？" },
		{ ELanguage::German,   "Ein Update ist verfügbar!\n\nMöchten Sie die Release-Seite\nbesuchen um es herunterzuladen?" },
		{ ELanguage::French,   "Une mise à jour est disponible !\n\nVoulez-vous visiter la page\ndes mises à jour pour la\ntélécharger ?" },
		{ ELanguage::Spanish,  "¡Hay una actualización disponible!\n\n¿Quieres ir a la página\npara descargarla?" },
		{ ELanguage::Italian,  "È disponibile un aggiornamento!\n\nVuoi visitare la pagina releases\nper scaricarlo?" }
	}
},
{
	"Video_BackendError",
	{
		{ ELanguage::English,  "Unable to create a D3D12 (Windows) or Vulkan backend.\n\nPlease make sure that:\n\n- Your system meets the minimum requirements.\n- Your GPU drivers are up to date.\n- Your operating system is on the latest version available." },
		{ ELanguage::Japanese, "D3D12（Windows）またはVulkanのバックエンドを作成できません。\n\n以下をご確認ください：\n\n※お使いのシステムが最小要件を満たしていること。\n※GPUドライバーが最新であること。\n※OSが利用可能な最新バージョンに更新されていること。" },
		{ ELanguage::German,   "Es ist nicht möglich, ein D3D12 (Windows) oder Vulkan-Backend zu erstellen.\n\nBitte stelle sicher, dass:\n\n- Dein System die Mindestanforderungen erfüllt.\n- Deine GPU-Treiber auf dem neuesten Stand sind.\n- Dein Betriebssystem auf der neuesten verfügbaren Version ist." },
		{ ELanguage::French,   "Impossible de créer un backend D3D12 (Windows) ou Vulkan.\n\nVeuillez vous assurer que :\n\n- Votre système répond aux critères minimums requis.\n- Les pilotes de votre processeur graphique sont à jour.\n- Votre système d'exploitation est à jour." },
		{ ELanguage::Spanish,  "No se ha podido crear un entorno de D3D12 (Windows) o de Vulkan.\n\nPor favor, asegúrate de que:\n\n- Tu equipo cumple con los requisitos mínimos.\n- Los drivers de tu tarjeta gráfica están actualizados.\n- Tu sistema operativo está actualizado a la última versión.\n" },
		{ ELanguage::Italian,  "Impossibile creare un backend D3D12 (Windows) o Vulkan.\n\nAssicurati che:\n\n- Il tuo sistema soddisfi i requisiti minimi.\n- I driver della scheda grafica siano aggiornati.\n- Il tuo sistema operativo sia aggiornato." }
	}
},
{
	"System_MemoryAllocationFailed",
	{
		{ ELanguage::English,  "Failed to allocate game memory.\n\nPlease make sure that:\n\n- You meet the minimum system requirements (8 GB).\n- Your page file is configured with at least 4-8 GB of virtual memory." },
		{ ELanguage::Japanese, "ゲーム用メモリを割り当てられませんでした。\n\n以下をご確認ください：\n\n※システムが最小要件（8 GB）を満たしていること。\n※ページファイルに少なくとも4～8 GBの仮想メモリが設定されていること。" },
		{ ELanguage::German,   "Fehler beim Zuweisen des Spielspeichers.\n\nBitte stelle sicher, dass:\n\n- Die Mindestanforderungen für das System erfüllt sind (8 GB).\n- Die Auslagerungsdatei mit mindestens 4-8 GB virtuellem Speicher konfiguriert ist." },
		{ ELanguage::French,   "Échec d'allocation de la mémoire du jeu.\n\nVeuillez vous assurer que :\n\n- Vous disposez de la configuration minimale requise (8 Go).\n- Votre fichier d’échange est configuré avec au moins 4 à 8 Go de mémoire virtuelle." },
		{ ELanguage::Spanish,  "Error al asignar memoria para el juego.\n\nPor favor, asegúrate de que:\n\n- Cumples los requisitos mínimos del sistema (8 GB).\n- Tu archivo de páginación está configurado con al menos 4 u 8 GB de memoria virtual." },
		{ ELanguage::Italian,  "Impossibile allocare la memoria per il gioco.\n\nAssicurati che:\n\n- Soddisfi i requisiti minimi di sistema (8 GB).\n- Il tuo file di paging sia configurato con almeno 4 o 8 GB di memoria virtuale." }
	}
},
{
	"IntegrityCheck_Success",
	{
		{ ELanguage::English,  "Installation check has finished.\n\nAll files seem to be correct.\n\nThe game will now close. Remove the launch argument to play the game." },
		{ ELanguage::Japanese, "インストールの確認が完了しました。\n\nすべてのファイルに問題はないようです。\n\nゲームを終了します。ゲームをプレイするには、起動引数を削除してください。" },
		{ ELanguage::German,   "Die Installation wurde überprüft.\n\nAlle Dateien scheinen korrekt zu sein.\n\nDas Spiel wird nun geschlossen. Entferne die Startoption, um das Spiel zu spielen." },
		{ ELanguage::French,   "La vérification de l'installation est terminée.\n\nTous les fichiers semblent corrects.\n\nL'application va maintenant se fermer. Retirez l'argument de lancement pour pouvoir lancer le jeu." },
		{ ELanguage::Spanish,  "La verificación de la instalación ha terminado.\n\nTodos los archivos parecen correctos.\n\nEl juego se cerrará ahora. Elimina el argumento de lanzamiento para jugar al juego." },
		{ ELanguage::Italian,  "La verifica dei file d'installazione è terminata.\n\nTutti i file sembrano corretti.\n\nIl gioco si chiuderà. Rimuovi l'argomento di avvio per poter giocare." }
	}
},
{
	"IntegrityCheck_Failed",
	{
		{ ELanguage::English,  "Installation check has failed.\n\nError: %s\n\nThe game will now close. Try reinstalling the game by using the --install launch argument." },
		{ ELanguage::Japanese, "インストールの確認に失敗しました。\n\nエラー：%s\n\nゲームを終了します。起動引数 --install を使用して、ゲームを再インストールしてみてください。" },
		{ ELanguage::German,   "Die Installationsprüfung ist fehlgeschlagen.\n\nFehler: %s\n\nDas Spiel wird nun geschlossen. Versuche das Spiel durch Verwendung der Startoption --install neu zu installieren." },
		{ ELanguage::French,   "La vérification de l'installation a échoué.\n\nErreur : %s\n\nL'application va maintenant se fermer. Essayez de réinstaller le jeu en utilisant l'argument de lancement --install." },
		{ ELanguage::Spanish,  "La verificación de la instalación ha fallado.\n\nError: %s\n\nEl juego se cerrará ahora. Intenta reinstalar el juego utilizando el argumento de lanzamiento --install." },
		{ ELanguage::Italian,  "La verifica dei file d'installazione non è andata a buon fine.\n\nErrore: %s\n\nIl gioco si chiuderà. Prova a reinstallare il gioco utilizzando l'argomento di avvio --install." }
	}
},
{
	"Common_On",
	{
		{ ELanguage::English,  "ON" },
		{ ELanguage::Japanese, "オン" },
		{ ELanguage::German,   "EIN" },
		{ ELanguage::French,   "OUI" },
		{ ELanguage::Spanish,  "SÍ" },
		{ ELanguage::Italian,  "SÌ" }
	}
},
{
	"Common_Off",
	{
		{ ELanguage::English,  "OFF" },
		{ ELanguage::Japanese, "オフ" },
		{ ELanguage::German,   "AUS" },
		{ ELanguage::French,   "NON" },
		{ ELanguage::Spanish,  "NO" },
		{ ELanguage::Italian,  "NO" }
	}
},
{
	"Common_Yes",
	{
		{ ELanguage::English,  "Yes" },
		{ ELanguage::Japanese, "はい" },
		{ ELanguage::German,   "Ja" },
		{ ELanguage::French,   "Oui" },
		{ ELanguage::Spanish,  "Sí" },
		{ ELanguage::Italian,  "Sì" }
	}
},
{
	"Common_No",
	{
		{ ELanguage::English,  "No" },
		{ ELanguage::Japanese, "いいえ" },
		{ ELanguage::German,   "Nein" },
		{ ELanguage::French,   "Non" },
		{ ELanguage::Spanish,  "No" },
		{ ELanguage::Italian,  "No" }
	}
},
{
	"Common_Ok",
	{
		{ ELanguage::English,  "OK" },
		{ ELanguage::Japanese, "OK" },
		{ ELanguage::German,   "OK" },
		{ ELanguage::French,   "OK" },
		{ ELanguage::Spanish,  "OK" },
		{ ELanguage::Italian,  "OK" }
	}
},
{
	"Common_Select",
	{
		{ ELanguage::English,  "select" },
		{ ELanguage::Japanese, "決定" },
		{ ELanguage::German,   "wählen" },
		{ ELanguage::French,   "confirmer" },
		{ ELanguage::Spanish,  "elegir" },
		{ ELanguage::Italian,  "seleziona" }
	}
},
{
	"Common_Back",
	{
		{ ELanguage::English,  "back" },
		{ ELanguage::Japanese, "もどる" },
		{ ELanguage::German,   "zurück" },
		{ ELanguage::French,   "retour" },
		{ ELanguage::Spanish,  "atrás" },
		{ ELanguage::Italian,  "indietro" }
	}
},
{
	"Common_Quit",
	{
		{ ELanguage::English,  "quit" },
		{ ELanguage::Japanese, "終了" },
		{ ELanguage::German,   "beenden" },
		{ ELanguage::French,   "quitter" },
		{ ELanguage::Spanish,  "salir" },
		{ ELanguage::Italian,  "esci" }
	}
},
{
	"Common_Cancel",
	{
		{ ELanguage::English,  "cancel" },
		{ ELanguage::Japanese, "キャンセル" },
		{ ELanguage::German,   "abbrechen" },
		{ ELanguage::French,   "annuler" },
		{ ELanguage::Spanish,  "cancelar" },
		{ ELanguage::Italian,  "annulla" }
	}
},
{
	"Common_Reset",
	{
		{ ELanguage::English,  "reset" },
		{ ELanguage::Japanese, "リセット" },
		{ ELanguage::German,   "zurücksetzen" },
		{ ELanguage::French,   "par défaut" },
		{ ELanguage::Spanish,  "reiniciar" },
		{ ELanguage::Italian,  "ripristina" }
	}
},
{
	"Common_Exit",
	{
		{ ELanguage::English,  "exit" },
		{ ELanguage::Japanese, "終了" },
		{ ELanguage::German,   "verlassen" },
		{ ELanguage::French,   "quitter" },
		{ ELanguage::Spanish,  "salir" },
		{ ELanguage::Italian,  "esci" }
	}
},
{
	"Common_Switch",
	{
		{ ELanguage::English,  "switch" },
		{ ELanguage::Japanese, "きりかえ" },
		{ ELanguage::German,   "wechseln" },
		{ ELanguage::French,   "changer" },
		{ ELanguage::Spanish,  "cambiar" },
		{ ELanguage::Italian,  "cambia" }
	}
},
{
	"Common_File",
	{
		{ ELanguage::English,  "File" },
		{ ELanguage::Japanese, "ファイル" },
		{ ELanguage::German,   "Datei" },
		{ ELanguage::French,   "Fichier" },
		{ ELanguage::Spanish,  "Archivo" },
		{ ELanguage::Italian,  "File" }
	}
},
{
	"Common_Install",
	{
		{ ELanguage::English,  "Install" },
		{ ELanguage::Japanese, "インストール" },
		{ ELanguage::German,   "Installieren" },
		{ ELanguage::French,   "Installer" },
		{ ELanguage::Spanish,  "Instalar" },
		{ ELanguage::Italian,  "Installa" }
	}
}
};

std::string& Localise(const std::string_view& key)
{
	auto localeFindResult = g_locale.find(key);
	if (localeFindResult != g_locale.end())
	{
		auto languageFindResult = localeFindResult->second.find(Config::Language);
		if (languageFindResult == localeFindResult->second.end())
			languageFindResult = localeFindResult->second.find(ELanguage::English);

		if (languageFindResult != localeFindResult->second.end())
			return languageFindResult->second;
	}

	return g_localeMissing;
}
