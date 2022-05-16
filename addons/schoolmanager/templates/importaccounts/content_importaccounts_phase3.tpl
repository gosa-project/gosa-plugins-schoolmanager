<input type="hidden" name="phase_03">
<br>
<h3>{t 1="3/11"}STEP %1: Check CSV data and assign to LDAP attributes{/t}</h3>

<p>
{t}Please assign the offered (LDAP) attributes to the CSV data columns. Note that
the CSV file has been rotated 90DEGREES counter-clockwise for better readability.{/t}
</p>
<br>
<br>
<table summary="{t}Check CSV data{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
{for $key=0 to $num_rows-1}
	<tr>
		<td bgcolor="#BBBBBB">
			<select name="column_head_{$key}" size="1" title="">
				{html_options options=$attrs selected=$attrs_selected[$key]}
			</select>
		</td>

		{foreach from=$data item=val2 key=key2}
			<td bgcolor="#EEEEEE">
				{$data[$key2][$key]}&nbsp;
			</td>
		{/foreach}

		{if $key == 0 && $data_size > 5}
			<td style="vertical-align: middle;" bgcolor="#EEEEEE"
				rowspan={$num_rows}>&nbsp;&nbsp;&nbsp;<b>… {$data_size}</b>&nbsp;&nbsp;&nbsp;
			</td>
		{/if}
	</tr>
{/for}
</table>