<template>
	<view class="setting-field">
		<text class="field-label">{{ label }}</text>
		<input
			class="field-input"
			:type="type"
			:password="password"
			:value="modelValue"
			:placeholder="placeholder"
			@input="$emit('update:modelValue', $event.detail.value)"
		/>
	</view>
</template>

<script>
export default {
	name: 'SettingField',
	props: {
		label: {
			type: String,
			required: true
		},
		modelValue: {
			type: [String, Number],
			default: ''
		},
		placeholder: {
			type: String,
			default: ''
		},
		type: {
			type: String,
			default: 'text'
		},
		password: {
			type: Boolean,
			default: false
		}
	},
	emits: ['update:modelValue']
}
</script>

<style>
.setting-field {
	display: flex;
	flex-direction: column;
	gap: 12rpx;
}

.field-label {
	font-size: 24rpx;
	font-weight: 700;
	color: #334155;
}

.field-input {
	height: 92rpx;
	padding: 0 26rpx;
	border-radius: 24rpx;
	background: #F8FAFC;
	border: 1rpx solid #E2E8F0;
	font-size: 28rpx;
	color: #0F172A;
	box-sizing: border-box;
}
</style>
