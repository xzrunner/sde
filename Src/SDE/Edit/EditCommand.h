#ifndef _IS_SDE_EDIT_EDIT_COMMAND_H_
#define _IS_SDE_EDIT_EDIT_COMMAND_H_

namespace IS_SDE
{
	namespace EDIT
	{
		enum EditCommandType
		{
			ECT_SHORTCUT
		};

		enum ShortcutKey
		{
			SK_ESC,
			SK_DELETE,
			SK_RESET
		};

		class EditCommand
		{
		public:
			EditCommandType type;

			union
			{
				ShortcutKey	shortcut;	// ECT_SHORTCUT
			};

		};	// EditCommand
	}
}

#endif // _IS_SDE_EDIT_EDIT_COMMAND_H_
