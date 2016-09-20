{if count($data_groups) > 0}
<br>
<b>{t}Standard (POSIX) groups{/t}:</p>

{t}The following list shows all POSIX groups managed by SchoolManager. You are about to empty all listed groups, that is drop all members from those groups.{/t}

<hr>
<br>

{foreach from=$data_groups item=group key=key}
<div style="float:left; width:24em;"
<div>
<table summary="{t 1=$group['cn'][0]}Group object: %1{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
	<tr>
	<td bgcolor="#BBBBBB" colspan="2">
		{t 1=$group['cn'][0]}Group object: %1{/t}
	</td>
	<td style="width:1em;">&nbsp;</td>
	</tr>
	{foreach from=$group item=valueArray key=attrDesc}
	{if in_array($attrDesc, array('cn','objectClass','description','memberUid'))}
	<tr>
		<td bgcolor="#EEEEEE">
		<b>{$attrDesc}:</b>
		</td>
		<td bgcolor="#FEFEFE">
		{foreach from=$valueArray item=value key=idx}
			{$value}<br />
		{/foreach}
		</td>
		<td tyle="width:1em;">&nbsp;</td>
	</tr>
	{/if}
	{/foreach}
</table>
<br>
</div>
{/foreach}
<div class="clear"></div>

<br>
<br>
{/if}

{if count($data_ogroups) > 0}
<b>{t}Object groups{/t}:</b>

{t}The following list shows all gosaGroupOfNames groups managed by SchoolManager. You are about to empty all listed groups, that is drop all members from those groups.{/t}

<hr>
<br>

{foreach from=$data_groups item=group key=key}
<div style="float:left; width:24em;"
<div>
<table summary="{t 1=$group['cn'][0]}Group object: %1{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
	<tr>
	<td bgcolor="#BBBBBB" colspan="2">
		{t 1=$group['cn'][0]}Group object: %1{/t}
	</td>
	<td style="width:1em;">&nbsp;</td>
	</tr>
	{foreach from=$group item=valueArray key=attrDesc}
	{if in_array($attrDesc, array('cn','objectClass','description','member'))}
	<tr>
		<td bgcolor="#EEEEEE">
		<b>{$attrDesc}:</b>
		</td>
		<td bgcolor="#FEFEFE">
		{foreach from=$valueArray item=value key=idx}
			{$value}<br />
		{/foreach}
		</td>
		<td tyle="width:1em;">&nbsp;</td>
	</tr>
	{/if}
	{/foreach}
</table>
<br>
</div>
{/foreach}
<div class="clear"></div>
{/if}