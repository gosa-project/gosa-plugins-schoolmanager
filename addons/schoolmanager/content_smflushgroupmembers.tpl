{if isset($cleanup_stats)}
<br>
<b>{t}Step 3:{/t} {t}Statistics{/t}:</b>

<p>
	{t}The clean-up process of SchoolManager managed groups has been finished now. SchoolManager is ready for importing a new school year, now.{/t}
</p>

<ul>
	<li>{t 1=$cleanup_stats['classes']}Emptied class groups: %1{/t}
	<li>{t 1=$cleanup_stats['courses']}Emptied course groups: %1{/t}
	<li>{t 1=$cleanup_stats['subjects']}Emptied subject groups: %1{/t}
	<li>{t 1=$cleanup_stats['parents']}Emptied parent groups: %1{/t}
</ul>

<table>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: top;">
			{t}Import teachers now...{/t}
		</td>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: bottom;">
			<button name="really_empty_schoolmanager_groups_now" type="submit">Proceed to the &quot;Manage Teachers&quot; SchoolManager module.</button>
		</td>
	</tr>
</table>
{/if}

{if count($data_groups) > 0}
<br>
<b>{t}Step 1:{/t} {t}Empty SchoolManager administered standard (POSIX) groups{/t}</b>

<p>
	{t}The following list shows all POSIX groups (classes, courses and teachers' subjects) managed by SchoolManager. You are about to empty all listed groups, i.e.: drop all members from those groups.{/t}
</p>

<p>
	{t}With the next import of teachers or students, many of these groups will get automatically re-populated with members. If you are at the beginning of a new school year, you want to do this!{/t}

<table>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: top;">
			{t}Flush all members from the listed course, class and subject (POSIX) groups now? After you have clicked this button, the process cannot be reverted!.{/t}
		</td>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: bottom;">
			<button name="really_empty_schoolmanager_groups_now" type="submit">Really empty all POSIX groups listed below, now!</button>
		</td>
	</tr>
</table>

<hr>
<br>

<table summary="{t 1=$group['cn'][0]}POSIX Group: %1{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
{foreach from=$data_groups item=group key=key}
	<tr>
	<th width="20%" bgcolor="#BBBBBB">
		{t}Group Name{/t}
	</th>
	<th width="30%"  bgcolor="#BBBBBB">
		{t}Description{/t}
	</th>
	<th width="*" bgcolor="#BBBBBB">
		{t}Members (to be removed){/t}
	</th>
	</tr>
	<tr>
		<td>{$group['cn'][0]}</td>
		<td>{$group['description'][0]}</td>
		<td>
		{foreach from=$group['memberUid'] item=value key=key}
			{$value}<br>
		{/foreach}
		</td>
	</tr>
{/foreach}
</table>
{/if}

{if count($data_ogroups) > 0}
<b>{t}Step 2:{/t} {t}Empty SchoolManager administered object groups{/t}:</b>

<p>
	{t}The following list shows all parent groups managed by SchoolManager. You are about to empty all listed groups, i.e.: drop all members from those groups.{/t}
</p>

<p>
	{t}With the next import of students (and their parents), many of these parent group will get automatically re-populated with members. At the beginning of a new school year, you want to do this!{/t}
</p>

<table>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: top;">
			{t}Flush all members from the listed parent (object) groups now? After you have clicked this button, the process cannot be reverted!.{/t}
		</td>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: bottom;">
			<button name="really_empty_schoolmanager_ogroups_now" type="submit">Really empty all parent (object) groups listed below, now!</button>
		</td>
	</tr>
</table>

<hr>
<br>

<table summary="{t 1=$group['cn'][0]}Group object: %1{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
{foreach from=$data_ogroups item=group key=key}
	<tr>
	<th bgcolor="#BBBBBB">
		{t}Group Name{/t}
	</th>
	<th bgcolor="#BBBBBB">
		{t}Description{/t}
	</th>
	<th bgcolor="#BBBBBB">
		{t}Members (to be removed){/t}
	</th>
	</tr>
	<tr>
		<td>{$group['cn'][0]}</td>
		<td>{$group['description'][0]}</td>
		<td>
		{foreach from=$group['member'] item=value key=key}
			{$value}<br>
		{/foreach}
		</td>
	</tr>
{/foreach}
</table>
{/if}
